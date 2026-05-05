// FriendProtocol.h
#pragma once

DWORD WINAPI FriendProcThread(LPVOID lpParam);
void FriendQueueThreadRun();

DWORD WINAPI MailProcThread(LPVOID lpParam);
void MailQueueThreadRun();