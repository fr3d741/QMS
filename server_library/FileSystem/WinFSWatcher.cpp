#include <FileSystem/WinFSWatcher.h>

#ifdef _WIN32
#include <CommonDefines.h>
#include <Utility/JsonNode.h>


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <thread>
#include <stop_token>

using namespace File_System;

void
WinFSWatcher::WatchDirectories(Logging::ILogger::Ptr /*logger*/, const std::vector<QString>& /*paths*/, IMessageQueue::Ptr /*queue*/) {

//    static std::thread _worker_thread;
//    static std::atomic<bool> _stop_not_requested;
//    static IMessageQueue::Ptr _queue;

//    _queue = queue;
//    _stop_not_requested.store(false);
//    if (_worker_thread.joinable())
//        _worker_thread.join(); //wait until finish

//    if (paths.empty())
//        return;

//    _stop_not_requested.store(true);
//    _worker_thread = std::thread([&]() {
//            DWORD dwWaitStatus;
//            std::vector<HANDLE> dwChangeHandles;
//            dwChangeHandles.reserve(paths.size());

//            // Watch the directory for file creation and deletion.

//            for (auto path : paths) {
//                dwChangeHandles.push_back(
//                    FindFirstChangeNotificationA(path.toLatin1().data(),                         // directory to watch
//                        TRUE,                         // do watch subtree
//                        FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME)); // watch file name changes
//            }

//            // Make a final validation check on our handles.

//            if (std::any_of(dwChangeHandles.begin(), dwChangeHandles.end(), [](HANDLE h) { return h == NULL; }))
//            {
//                printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
//                ExitProcess(GetLastError());
//            }

//            // Change notification is set. Now wait on both notification
//            // handles and refresh accordingly.

//            while (_stop_not_requested.load())
//            {
//                // Wait for notification.

//                dwWaitStatus = WaitForMultipleObjects(static_cast<DWORD>(dwChangeHandles.size()), dwChangeHandles.data(), FALSE, 1000);

//                switch (dwWaitStatus)
//                {
//                case WAIT_TIMEOUT:

//                    // A timeout occurred, this would happen if some value other
//                    // than INFINITE is used in the Wait call and no changes occur.
//                    // In a single-threaded environment you might not want an
//                    // INFINITE wait.
//                    break;

//                case WAIT_OBJECT_0:
//                default:
//                    // A file was created, renamed, or deleted in the directory.
//                    // Refresh this directory and restart the notification.
//                    auto json = JsonNode::Create(logger);
//                    json->Add(KeyWords(Keys::NodeType), static_cast<int>(Tags::Path_Update));
//                    json->Add(TagWords(Tags::Path_Update), paths.at(dwWaitStatus));
//                    _queue->Add(json->ToString());
//                    if (FindNextChangeNotification(dwChangeHandles[dwWaitStatus]) == FALSE)
//                    {
//                        printf("\n ERROR: FindNextChangeNotification function failed.\n");
//                        ExitProcess(GetLastError());
//                    }
//                    break;
//                }
//            }
//        });
}
#endif
