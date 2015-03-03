#Notas sobre mi trabajo respecto a Bluetooth Serial Port en Firefox OS



## Stack


Hay 4 implementaciones distintas del stack de Bluetooth en FxOS.


Se dividen según el subsistema de Bluetooth que usen:

- **BlueZ**: Usado generalmente en las distribuciones Linux

- **Bluedroid**: Implementación propia de AOSP con bastante apoyo de Qualcomm. El objetivo ya conseguido es migrar a Bluedroid, pero cuando yo estaba con el tema sólo había un par de devices (Flame y Open C) que se podían compilar con KitKat, que es el requisito para usar Bluedroid.


Y se dividen, otra vez, según la API de Bluetooth (si v1 o v2), haciendo que finalmente nos encontremos ante 4 codebases distintas.



## Builds


Por defecto me encontré que Gecko hace la build con BlueZ y v1, que es justo lo que no quería, ya que todo se estaba migrando a Bluedroid y v2. Aunque cuando yo estaba trabajando la v2 directamente no funcionaba.

Ahora mismo lo más probable de todas formas es que ya compile Bluedroid y v2 por defecto, ya que supongo que la transición de BlueZ y v1 ya esté más que acabada.



## SVN/Git


Estando acostumbrado a Git me horroricé al trabajar con SVN. Uno de los problemas que tenía era a la hora de hacer los merges, que a menudo acababa haciendo a mano con la consecuente pérdida de tiempo.

Al final, cuando me enteré de la existencia del repo Git, la vida se me hizo bastante más fácil, me habría gustado saberlo desde el principio.



## Master


Lo mejor claramente es freezear una versión que funcione y trabajar sobre ella, y cuando todo funcione ver los cambios en master y adaptarlo.

Perdí muchísimo tiempo por refrescar master, porque cada dos días todo se rompía. El problema era que una vez que cometes en error de hacer pull, te encuentras con que has pulleado decenas o cientos de commits así que cuando algo deja de funcionar tienes que ir revirtiendo (y buildeando, y probando) hasta que encuentras la revisión en la que se fastidió.

Teniendo en cuenta que por build después de hacer pull (que aunque sigue sin ser una compilación total hay muchos cambios) se pierden unos 10-30min, se te puede ir el día revirtiendo y probando tan sólo 10 commits.



## Testing


El gran problema de probar algo que no está implementado y para lo que se requieren dos dispositivos (para realizar la conexión) es que uno de los dispositivos debe tener implementado Serial Port y debes conocer su stack.

En mi caso me pasé bastante tiempo entendiendo cómo emular puertos serial en Linux para poder usar mi laptop como el otro device al que se conectaría FxOS.

Para eso cree un set de herramientas en [este repo](https://github.com/luisivan/bt-serialport-fxos).

Por ejemplo, [este script de Node](https://github.com/luisivan/bt-serialport-fxos/blob/master/btsp) abre un Serial Port bajo */dev/rfcomm0*, y loggea todo lo que recibe a parte de enviar siempre los mismos datos cuando recibe algo.

Antes de ejecutarlo hay que bindear el puerto rfcomm con el comando *rfcomm bind 0 address 1*.

Por otro lado, [esta app](https://github.com/luisivan/bt-serialport-fxos/tree/master/testapp) es para probar la API que construí de BTSP.

Lo único que hace es conseguir la lista de devices paireados, y les envía datos por SP a todos. También se pone a la escucha y alerta si recibe algo.



## BlueZ 5


Había bastantes ocasiones en las que no me funcionaba la conexión sin motivo aparente pero no estaba seguro de la razón. Incluso Gecko crasheaba reiniciando FxOS y provocando bootloops.

Después de semanas de locura, acabé recurriendo a [Shawn Huang](https://mozillians.org/en-US/u/shuang/), y juntos conseguimos llegar a la conclusión de que era un bug nunca visto antes en Gecko que le impedía conectarse con cualquier device con la stack BlueZ 5, que era el caso de mi Arch Linux. BlueZ 4 (Ubuntu 12.04 por ejemplo) funcionaba perfectamente.

Me habría gustado darme cuenta de esto unos meses antes. Ni se me pasó por la cabeza que un bug así estuviese en master y nadie se hubiese dado cuenta.



## Código


Me pareció bastante aburrido tener que lidiar con los 20.000 tipos que hay en Gecko para todo. Hay como 20 tipos de NSBlablablaString, y lo mismo con todo lo demás. Teóricamente hay una metodología de uso para cada uno de esos tipos, pero en la práctica lo hace todo bastante caótico.

Por otro lado, ya que me pilló justo en la migración tanto de un stack a otro como de la v1 a la v2, el código cambiaba cada día y muchas veces de forma tan importante que requería cambios en mi módulo de Serial Port.

Afortunadamente el módulo de Object Push comparte bastantes cosillas con el purpose del SP así que me sirvió de referencia en todo momento.

Otro tema es que los módulos base, como BluetoothSocket, están adaptados sólo a las features que hay ahora y no a las que podría haber, así que me vi obligado a modificar algunos módulos que no me correspondían.



## Gente


- **Shawn Huang** (shawnjohnjr en el IRC, shuang@mozilla.com): Muy crack y dispuesto a ayudar. Está en Taipei, así que hay algo de cambio horario pero suele responder muy rápido. Ha trabajado en A2DP y con Bluetooth LE.

- **Thomas Zimmermann** (tzimmermann en el IRC, tzimmermann@mozilla.com): Thomas sí que trabaja desde Europa así que hay menos cambio horario, pero tarda mucho en responder a veces y no parece tener tanto tiempo como Shawn para echar un cable.

- **Ben Tian** (btian@mozilla.com): Encargado de la API de Bluetooth. Cualquier idea a la hora de modificar la API o crear nuevos métodos teóricamente debe ser hablada con él.



## Notas interesantes


Nunca había tocado Bluetooth antes, así que me tocó empaparme bastante del protocolo. La verdad que viniendo de conocer algo mejor WiFi Direct, Bluetooth me pareció algo arcaico. Aunque he estado mirando Bluetooth LE últimamente y me parece mucho más simple y elegante.

De todas formas, [aquí](https://github.com/luisivan/bt-serialport-fxos/blob/master/links.md) están algunos de los links con los que me documenté.


También tomé algunas notas [aquí](https://github.com/luisivan/bt-serialport-fxos/blob/master/notes.md) sobre el procedimiento de la conexión, el protocolo, GEcko, Arduino etc.


Finalmente, la lista de [los archivos importantes que hay que tocar](https://github.com/luisivan/bt-serialport-fxos/blob/master/files.md)



## TODO


Me hice una [lista de tareas por hacer](https://github.com/luisivan/bt-serialport-fxos/blob/master/todo.md) y básicamente me quedé parado con un par de problemas:

- Exponer el servicio de forma que los otros devices se den cuenta de que el servicio SP está escuchando

- Solventar un problema por el cual el socket se conectaba pero crasheaba inmediatamente al establecer la conexión


Son dos problemas complicados simplemente por lo horrible que es debuggear algo tan low level. Aunque después de un tiempo conseguí ir descifrando los logs de errores de Bluetooth, y en mi repo tengo algunos de esos logs. [Aquí](https://github.com/luisivan/bt-serialport-fxos/blob/master/errorlog.txt) uno donde asocio números de errores con su respectivo significado (líneas 30 a 44 y 77 a 90).


