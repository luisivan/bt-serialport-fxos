/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/basictypes.h"
#include "BluetoothSppManager.h"

#include "BluetoothService.h"
#include "BluetoothSocket.h"
#include "BluetoothProfileController.h"
//#include "BluetoothUtils.h"
#include "BluetoothUuid.h"
#include "mozilla/dom/bluetooth/BluetoothTypes.h"
#include "mozilla/Services.h"
#include "mozilla/StaticPtr.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"

USING_BLUETOOTH_NAMESPACE
using namespace mozilla;
using namespace mozilla::ipc;

namespace {

StaticRefPtr<BluetoothSppManager> sBluetoothSppManager;
static bool sInShutdown = false;
}

NS_IMETHODIMP
BluetoothSppManager::Observe(nsISupports* aSubject,
                             const char* aTopic,
                             const char16_t* aData)
{
  MOZ_ASSERT(sBluetoothSppManager);

  if (!strcmp(aTopic, NS_XPCOM_SHUTDOWN_OBSERVER_ID)) {
    HandleShutdown();
    return NS_OK;
  }

  MOZ_ASSERT(false, "BluetoothSppManager got unexpected topic!");
  return NS_ERROR_UNEXPECTED;
}

static const uint8_t UUID_SERIAL_PORT[] = {
  0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00,
  0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
};

BluetoothSppManager::BluetoothSppManager() : mConnected(false)
                                           , mIsServer(true)
{
  mDeviceAddress.AssignLiteral(BLUETOOTH_ADDRESS_NONE);
}

BluetoothSppManager::~BluetoothSppManager()
{
  nsCOMPtr<nsIObserverService> obs = services::GetObserverService();
  NS_ENSURE_TRUE_VOID(obs);
  if (NS_FAILED(obs->RemoveObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID))) {
    BT_WARNING("Failed to remove shutdown observer!");
  }
}

bool
BluetoothSppManager::Init()
{
  nsCOMPtr<nsIObserverService> obs = services::GetObserverService();
  NS_ENSURE_TRUE(obs, false);
  if (NS_FAILED(obs->AddObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID, false))) {
    BT_WARNING("Failed to add shutdown observer!");
    return false;
  }

  /**
   * We don't start listening here as BluetoothServiceBluedroid calls Listen()
   * immediately when BT stops.
   *
   * If we start listening here, the listening fails when device boots up since
   * Listen() is called again and restarts server socket. The restart causes
   * absence of read events when device boots up.
   */

  return true;
}

//static
BluetoothSppManager*
BluetoothSppManager::Get()
{
  MOZ_ASSERT(NS_IsMainThread());

  // If sBluetoothSppManager already exists, exit early
  if (sBluetoothSppManager) {
    return sBluetoothSppManager;
  }

  // If we're in shutdown, don't create a new instance
  NS_ENSURE_FALSE(sInShutdown, nullptr);

  // Create a new instance, register, and return
  BluetoothSppManager *manager = new BluetoothSppManager();
  NS_ENSURE_TRUE(manager->Init(), nullptr);

  sBluetoothSppManager = manager;
  return sBluetoothSppManager;
}

void
BluetoothSppManager::ConnectInternal(const nsAString& aDeviceAddress)
{
  MOZ_ASSERT(NS_IsMainThread());

  mConnected = true;
  // Stop listening because currently we only support one connection at a time.
  if (mServerSocket) {
    //mServerSocket->CloseSocket();
    mServerSocket = nullptr;
  }

  mIsServer = false;

  BluetoothService* bs = BluetoothService::Get();
  if (!bs || sInShutdown || mSocket) {
    OnSocketConnectError(mSocket);
    return;
  }

  mSocket =
    new BluetoothSocket(this, BluetoothSocketType::RFCOMM, false, true);

  BT_LOGR("Connecting to device");

  mSocket->ConnectSocket(aDeviceAddress, UUID_SERIAL_PORT, -1);
}

void
BluetoothSppManager::HandleShutdown()
{
  MOZ_ASSERT(NS_IsMainThread());
  sInShutdown = true;
  Disconnect(nullptr);
  sBluetoothSppManager = nullptr;
}

bool
BluetoothSppManager::Listen()
{
  BT_LOGR("Listen");

  MOZ_ASSERT(NS_IsMainThread());

  if (mSocket) {
    BT_WARNING("mSocket exists. Failed to listen.");
    return false;
  }

  BT_LOGR("Listen 2");

  /**
   * Restart server socket since its underlying fd becomes invalid when
   * BT stops; otherwise no more read events would be received even if
   * BT restarts.
   */
  if (mServerSocket) {
    //mServerSocket->CloseSocket();
    mServerSocket = nullptr;
  }

  mServerSocket =
    new BluetoothSocket(this, BluetoothSocketType::RFCOMM, false, true);

  // BluetoothReservedChannels::RFCOMM
  // Listen 0 to auto listen?
  if (!mServerSocket->ListenSocket(UUID_SERIAL_PORT, -1)) {
    BT_WARNING("[SPP] Can't listen on RFCOMM socket!");
    mServerSocket = nullptr;
    return false;
  }

  BT_LOGR("Listen 3");

  mIsServer = true;

  return true;
}

bool
BluetoothSppManager::SendData(const nsAString& aDeviceAddress,
                              const nsAString& aData)
{
    BT_LOGR("[SPP] SENDING DATA");
    nsCString aStr = NS_ConvertUTF16toUTF8(aData);
    //BT_LOGR(aStr);
    UnixSocketRawData* s = new UnixSocketRawData(aStr.get(), sizeof(aStr));

    if (!mSocket) {
      BT_WARNING("mSocket does not exist. Gonna connect now.");
      ConnectInternal(aDeviceAddress);
      return false;
    }

    mSocket->SetAddress(aDeviceAddress);
    BT_LOGR("[SPP] BOOOO");
    // y si se peta porque el socket es null?
    //mSocket->SendSocketData(s);

    return true;
}


// READY
// Implementation of BluetoothSocketObserver
void
BluetoothSppManager::ReceiveSocketData(BluetoothSocket* aSocket,
                                       nsAutoPtr<UnixSocketRawData>& aMessage)
{
  /*nsAutoCString msg((const char*)aMessage->mData.get(), aMessage->mSize);
  msg.StripWhitespace();
  BT_LOGR("[COOLEST SHIT ON EARTH]");
  BT_LOGR("[%s]", msg.get());*/
}

void
BluetoothSppManager::OnSocketConnectSuccess(BluetoothSocket* aSocket)
{
  BT_LOGR("WAKALAYAKOO");
  BT_LOGR("[%s]", (mIsServer)? "server" : "client");
  MOZ_ASSERT(aSocket);

  /**
   * If the created connection is an inbound connection, close server socket
   * because currently only one file-transfer session is allowed. After that,
   * we need to make sure that server socket would be nulled out.
   * As for outbound connections, we just notify the controller that it's done.
   */
  if (aSocket == mServerSocket) {
    MOZ_ASSERT(!mSocket);
    mServerSocket.swap(mSocket);
  }

  // Cache device address since we can't get socket address when a remote
  // device disconnect with us.
  mSocket->GetAddress(mDeviceAddress);
}

void
BluetoothSppManager::OnSocketConnectError(BluetoothSocket* aSocket)
{
  BT_LOGR("WAKALAYAKA");
  BT_LOGR("[%s]", (mIsServer) ? "server" : "client");

  mServerSocket = nullptr;
  mSocket = nullptr;

  Listen();
}

void
BluetoothSppManager::OnSocketDisconnect(BluetoothSocket* aSocket)
{
  BT_LOGR("WAKALAYAKAkakakakkakakaka");
  MOZ_ASSERT(aSocket);
  if (aSocket != mSocket) {
    // Do nothing when a listening server socket is closed.
    return;
  }
  BT_LOGR("[%s]", (mIsServer) ? "server" : "client");

  mConnected = false;
  mDeviceAddress.AssignLiteral(BLUETOOTH_ADDRESS_NONE);

  mSocket = nullptr;

  Listen();
}

// Implementation of BluetoothProfileManagerBase
NS_IMPL_ISUPPORTS(BluetoothSppManager, nsIObserver)

void
BluetoothSppManager::OnGetServiceChannel(const nsAString& aDeviceAddress,
                                         const nsAString& aServiceUuid,
                                         int aChannel)
{
    BT_WARNING("%s: OnGetServiceChannel was called", __FUNCTION__);
}

void
BluetoothSppManager::OnUpdateSdpRecords(const nsAString& aDeviceAddress) {}

void
BluetoothSppManager::GetAddress(nsAString& aDeviceAddress)
{
  return mSocket->GetAddress(aDeviceAddress);
}

bool
BluetoothSppManager::IsConnected()
{
  return mConnected;
}

void
BluetoothSppManager::Connect(const nsAString& aDeviceAddress,
                             BluetoothProfileController* aController)
{
    MOZ_ASSERT(NS_IsMainThread());
    MOZ_ASSERT(aController);

    if (sInShutdown) {
      aController->NotifyCompletion(NS_LITERAL_STRING(ERR_NO_AVAILABLE_RESOURCE));
      return;
    }

    ConnectInternal(aDeviceAddress);
}

void
BluetoothSppManager::Disconnect(BluetoothProfileController* aController)
{
  if (mSocket) {
    //mSocket->CloseSocket();
  } else {
    BT_WARNING("%s: No ongoing transfer to stop", __FUNCTION__);
  }
}

void
BluetoothSppManager::OnConnect(const nsAString& aErrorStr)
{
    BT_WARNING("%s: fuckfuck", __FUNCTION__);
}

void
BluetoothSppManager::OnDisconnect(const nsAString& aErrorStr)
{
    BT_WARNING("%s: fuckfuck", __FUNCTION__);
}

void
BluetoothSppManager::Reset() {}
