#include "stdafx.h"
#include "Thread.h"
#include "WzQueue.h"

// ----------------------------------------------------------------------
// Friend Queue Thread
// ----------------------------------------------------------------------

DWORD WINAPI FriendProcThread(LPVOID lpParam)
{
	int ExitThread = 0;

	BYTE headcode[9];
	int Index[3];
	DWORD nSize[3];

	BYTE buf[1024];

	do
	{
		if (FriendQueue.GetFromQueue(buf, nSize, headcode, Index) == 1)
		{
			switch (headcode[0])
			{
			case 0x02:
				DGFriendOfflineSend((char*)buf);
				cLog.AddTD("Friend LogOut Queue Name: ( %s )", (char*)buf);
				break;

			case 0x60:
				GDFrinedListReq((FHP_FRIENDLIST_REQ*)buf, Index[0]);

				if (FriendQueue.GetCount() > 2)
				{
					cLog.AddTD("Friend Login Queue : %d",FriendQueue.GetCount());
				}
				break;

			case 0x62:
				GDFriendStateChange((FHP_FRIEND_STATE_C*)buf, Index[0]);

				if (FriendQueue.GetCount() > 2)
				{
					cLog.AddTD("Friend State Queue : %d",FriendQueue.GetCount());
				}
				break;

			case 0x63:
				GDFriendAddReq((FHP_FRIEND_ADD_REQ*)buf, Index[0]);

				if (FriendQueue.GetCount() > 2)
				{
					cLog.AddTD("Friend Add Queue : %d",FriendQueue.GetCount());
				}
				break;

			case 0x64:
				GDWaitFriendAddReq((FHP_WAITFRIEND_ADD_REQ*)buf, Index[0]);

				if (FriendQueue.GetCount() > 2)
				{
					cLog.AddTD("Friend WaitFriend Add Queue : %d",FriendQueue.GetCount());
				}
				break;

			case 0x65:
				GDFriendDelReq((FHP_FRIEND_ADD_REQ*)buf, Index[0]);

				if (FriendQueue.GetCount() > 2)
				{
					cLog.AddTD("Friend Del Queue : %d",FriendQueue.GetCount());
				}
				break;

			case 0xFF:
				ExitThread = 1;
				break;

			default:
				cLog.AddTD("error-L2 : Friend Not Queue : (%x) %d",headcode[0],FriendQueue.GetCount());
				break;
			}
		}

		Sleep(1);
	}
	while (ExitThread != 1);
	return 1;
}

// ----------------------------------------------------------------------
// Friend Queue Startup
// ----------------------------------------------------------------------

void FriendQueueThreadRun()
{
	DWORD ThreadID = 0;
	gFriendQueueThreadHandle = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(FriendProcThread), NULL, 0, &ThreadID);
}

// ----------------------------------------------------------------------
// Mail Queue Thread
// ----------------------------------------------------------------------

DWORD WINAPI MailProcThread(LPVOID lpParam)
{
    int ExitThread = 0;

    BYTE headcode[9];
    int uindex[3];
    DWORD pSize[3];

    FHP_FRIEND_MEMO_SEND pObject[7];

    do
    {
        if (MailQueue.GetFromQueue((BYTE*)&pObject[0].h, pSize, headcode, uindex) == 1)
        {
            switch (headcode[0])
            {
            case 0x70:
            {
                GDFriendMemoSend((FHP_FRIEND_MEMO_SEND*)pObject,uindex[0]);

                if (MailQueue.GetCount() > 1)
                {
                    cLog.AddC(LOGC_GREEN,"Friend Mail Send Queue %d",MailQueue.GetCount());
                }

            } break;

            case 0x71:
            {
                GDFriendMemoListReq((FHP_FRIEND_MEMO_LIST_REQ*)pObject,uindex[0]);

                if (MailQueue.GetCount() > 1)
                {
                    cLog.AddC(LOGC_GREEN,"Friend Mail List Queue %d",MailQueue.GetCount());
                }

            } break;

            case 0x72:
            {
                GDFriendMemoRead((FHP_FRIEND_MEMO_RECV_REQ*)pObject,uindex[0]);

                if (MailQueue.GetCount() > 1)
                {
                    cLog.AddC(LOGC_GREEN,"Friend Mail Read Queue %d",MailQueue.GetCount());
                }

            } break;

            case 0x73:
            {
                GDFriendMemoDel((FHP_FRIEND_MEMO_DEL_REQ*)pObject,uindex[0]);

                if (MailQueue.GetCount() > 1)
                {
                    cLog.AddC(LOGC_GREEN,"Friend Mail Del Queue %d",MailQueue.GetCount());
                }

            } break;

            case 0xFF:
            {
                ExitThread = 1;

            } break;

            default:
            {
                cLog.AddC(LOGC_GREEN,"error-L2 : Mail Not Queue : (%x) %d",headcode[0],MailQueue.GetCount());

            }
            	break;
            }
        }

        Sleep(1);

    } while (ExitThread != 1);

    return 1;
}

// ----------------------------------------------------------------------
// Mail Queue Startup
// ----------------------------------------------------------------------

void MailQueueThreadRun()
{
    DWORD ThreadID = 0;

    gMailQueueThreadHandle =CreateThread(NULL,0,LPTHREAD_START_ROUTINE(MailProcThread),NULL,0,&ThreadID);
}