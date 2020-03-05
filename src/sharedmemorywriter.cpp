/*****************************************************************************
* Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "sharedmemorywriter.h"

#include "xpconnect.h"
#include "fs/sc/xpconnecthandler.h"

#include <QBuffer>
#include <QDataStream>

SharedMemoryWriter::SharedMemoryWriter()
{
  qDebug() << Q_FUNC_INFO;
  xpConnect = new xpc::XpConnect();
  xpConnect->initDataRefs();
}

SharedMemoryWriter::~SharedMemoryWriter()
{
  qDebug() << Q_FUNC_INFO;
  delete xpConnect;
}

void SharedMemoryWriter::fetchAndWriteData(bool fetchAi)
{
  {
    QMutexLocker locker(&dataMutex);
    if(!xpConnect->fillSimConnectData(data, fetchAi))
      data = atools::fs::sc::EMPTY_SIMCONNECT_DATA;
  }

  waitCondition.wakeAll();
}

void SharedMemoryWriter::terminateThread()
{
  terminate = true;
  waitCondition.wakeAll();
  wait();
}

void SharedMemoryWriter::writeData(const QByteArray& simDataBytes, bool terminated)
{
  QByteArray allBytes;
  QDataStream stream(&allBytes, QIODevice::WriteOnly);
  stream << static_cast<quint32>(static_cast<quint32>(simDataBytes.size()) + sizeof(quint32) * 2);
  stream << static_cast<quint32>(terminated);
  stream.writeRawData(simDataBytes.constData(), static_cast<int>(simDataBytes.size()));

  if(allBytes.size() > atools::fs::sc::SHARED_MEMORY_SIZE)
    qWarning() << "LittleXpconnect" << Q_FUNC_INFO
               << "Data too large" << allBytes.size() << ">" << atools::fs::sc::SHARED_MEMORY_SIZE;
  else
  {
    if(sharedMemory.lock())
    {
      memcpy(sharedMemory.data(), allBytes.constData(), static_cast<size_t>(allBytes.size()));
      // qDebug() << "Lock ok size" << allBytes.size();
      sharedMemory.unlock();
    }
    else
      qInfo() << "LittleXpconnect" << Q_FUNC_INFO << "Cannot lock" << sharedMemory.key()
              << "native" << sharedMemory.nativeKey();
  }
}

void SharedMemoryWriter::run()
{
  qDebug() << "LittleXpconnect" << Q_FUNC_INFO;

  sharedMemory.setKey(atools::fs::sc::SHARED_MEMORY_KEY);
  if(!sharedMemory.create(atools::fs::sc::SHARED_MEMORY_SIZE, QSharedMemory::ReadWrite))
  {
    qWarning() << "LittleXpconnect" << Q_FUNC_INFO << "Cannot create" << sharedMemory.errorString();

    if(!sharedMemory.attach(QSharedMemory::ReadWrite))
      qWarning() << "LittleXpconnect" << Q_FUNC_INFO << "Cannot attach" << sharedMemory.errorString();
    else
      qInfo() << "LittleXpconnect" << Q_FUNC_INFO << "Attached to" << sharedMemory.key()
              << "native" << sharedMemory.nativeKey();
  }
  else
    qInfo() << "LittleXpconnect" << Q_FUNC_INFO << "Created" << sharedMemory.key()
            << "native" << sharedMemory.nativeKey();

  waitMutex.lock();

  while(true)
  {
    waitCondition.wait(&waitMutex);

    QByteArray simDataBytes;
    QBuffer buffer(&simDataBytes);
    buffer.open(QIODevice::WriteOnly);

    {
      QMutexLocker locker(&dataMutex);
      data.write(&buffer);
    }

    buffer.close();

    if(terminate)
    {
      writeData(simDataBytes, terminate);
      break;
    }
    else
      writeData(simDataBytes, false);
  }
  waitMutex.unlock();
  qDebug() << "LittleXpconnect" << Q_FUNC_INFO << "terminate" << terminate;

  if(!sharedMemory.detach())
    qWarning() << "Cannot detach" << sharedMemory.errorString() << "from" << sharedMemory.key()
               << "native" << sharedMemory.nativeKey();
  else
    qInfo() << "LittleXpconnect" << Q_FUNC_INFO << "Detached from" << sharedMemory.key()
            << "native" << sharedMemory.nativeKey();
}
