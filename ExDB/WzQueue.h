#ifndef __WZQUEUE_H__
#define __WZQUEUE_H__

typedef struct _ListNode
{
	_ListNode* pUpLink;
	_ListNode* pDownLink;
	LPBYTE pObject;
	DWORD nSize;
	BOOL bSending;
	int nOfs;
	BYTE headcode;
	int uindex;
} ListNode;

#define MAX_NODE		1280
#define SPIN_COUNT		4000

class  WZQueue
{
public:
	WZQueue(DWORD nMaxNode = MAX_NODE);
	virtual ~WZQueue();

	DWORD GetCount();
	BOOL IsEmpty();
	BOOL AddToQueue(const LPBYTE pObject, DWORD nSize, BYTE headcode, int uindex);
	ListNode* AddToQueueList(const LPBYTE pObject, DWORD nSize, BYTE headcode, int uindex);
	BOOL GetFromQueue(LPBYTE pObject, LPDWORD pSize, LPBYTE headcode, int* uindex);
	void Init();
	void Free();

	BOOL StartQ();
	BOOL NextQ();
	ListNode* GetCurData(LPBYTE pObject, LPDWORD pSize, LPBYTE headcode, int* uindex);
	BOOL Pop(ListNode* pCur, LPBYTE pObject, int nOfs, int* nSize, int* sentbytes);
	void DeleteNode(ListNode* pCur);
	void SetCurDataSending(ListNode* pNode);


private:
	ListNode* GetHeadNode();
	BOOL AddTail(ListNode* pNewNode);
	void DeleteAllNodes();

	DWORD m_Count;
	DWORD m_Max_Node;
	ListNode* m_pHead;
	ListNode* m_pTail;
	ListNode* m_pCur;

	CRITICAL_SECTION m_CriticalSection;
};

#endif
