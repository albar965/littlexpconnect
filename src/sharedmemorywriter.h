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

#ifndef SHAREDMEMORYWRITERTHREAD_H
#define SHAREDMEMORYWRITERTHREAD_H

#include "fs/sc/simconnectdata.h"

#include <QMutex>
#include <QSharedMemory>
#include <QThread>
#include <QWaitCondition>

/*
 * Use a background thread to write the data to the shared memory to avoid simulator stutters due to
 * locking
 */
class SharedMemoryWriter :
  public QThread
{
public:
  SharedMemoryWriter();
  virtual ~SharedMemoryWriter();

  /* Fetch data from the datarefs (main thread context) and pass it over to the
   * shared memory writer (writing in this thread's context) */
  void fetchAndWriteData(bool fetchAi);

  /* Send termination signal and wait for terminated */
  void terminateThread();

private:
  virtual void run() override;
  void writeData(const QByteArray& simDataBytes, bool terminated);

  bool terminate = false;
  atools::fs::sc::SimConnectData data;

  /* Syncronize SimConnectData access */
  QMutex dataMutex;

  /* Wakes thread up once new data has arrived */
  QMutex waitMutex;
  QWaitCondition waitCondition;

  /* Shared memory for local communication */
  QSharedMemory sharedMemory;

  xpc::XpConnect *xpConnect = nullptr;

};

#endif // SHAREDMEMORYWRITERTHREAD_H
