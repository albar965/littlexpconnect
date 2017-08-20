/*****************************************************************************
* Copyright 2015-2017 Alexander Barthel albar965@mailbox.org
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

#include <QBuffer>
#include <QDataStream>

SharedMemoryWriter::SharedMemoryWriter()
{

}

SharedMemoryWriter::~SharedMemoryWriter()
{

}

void SharedMemoryWriter::fetchAndWriteData(bool fetchAi)
{
  {
    QMutexLocker locker(&dataMutex);
    if(!xpc::XpConnect::fillSimConnectData(data, fetchAi))
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

  if(allBytes.size() > SHARED_MEMORY_SIZE)
    qWarning() << "LittleXpConnect" << Q_FUNC_INFO
               << "Data too large" << allBytes.size() << ">" << SHARED_MEMORY_SIZE;
  else
  {
    if(sharedMemory.lock())
    {
      memcpy(sharedMemory.data(), allBytes.constData(), static_cast<size_t>(allBytes.size()));
      // qDebug() << "Lock ok size" << allBytes.size();
      sharedMemory.unlock();
    }
    else
      qInfo() << "LittleXpConnect" << Q_FUNC_INFO << "Cannot lock" << sharedMemory.key()
              << "native" << sharedMemory.nativeKey();
  }
}

void SharedMemoryWriter::run()
{
  qDebug() << "LittleXpConnect" << Q_FUNC_INFO;

  sharedMemory.setKey("LittleXpConnect");
  if(!sharedMemory.create(SHARED_MEMORY_SIZE, QSharedMemory::ReadWrite))
  {
    qWarning() << "LittleXpConnect" << Q_FUNC_INFO << "Cannot create" << sharedMemory.errorString();

    if(!sharedMemory.attach(QSharedMemory::ReadWrite))
      qWarning() << "LittleXpConnect" << Q_FUNC_INFO << "Cannot attach" << sharedMemory.errorString();
    else
      qInfo() << "LittleXpConnect" << Q_FUNC_INFO << "Attached to" << sharedMemory.key()
              << "native" << sharedMemory.nativeKey();
  }
  else
    qInfo() << "LittleXpConnect" << Q_FUNC_INFO << "Created" << sharedMemory.key()
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
  qDebug() << "LittleXpConnect" << Q_FUNC_INFO << "terminate" << terminate;

  if(!sharedMemory.detach())
    qWarning() << "Cannot detach" << sharedMemory.errorString() << "from" << sharedMemory.key()
               << "native" << sharedMemory.nativeKey();
  else
    qInfo() << "LittleXpConnect" << Q_FUNC_INFO << "Detached from" << sharedMemory.key()
            << "native" << sharedMemory.nativeKey();
}
