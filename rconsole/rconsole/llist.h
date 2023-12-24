
#if !defined(_LLIST_H)
#define _LLIST_H


typedef struct s_LList {
	struct s_LList *pPrev, *pNext;
} LList;




inline void LListInit(LList *pHead) {

	pHead->pPrev = pHead->pNext = pHead;
}

inline int LListEmpty(LList *pHead) {

	return pHead->pNext == pHead;
}

inline void LListAdd(LList *pNew, LList *pPrev, LList *pNext) {

	pNext->pPrev = pNew;
	pNew->pNext = pNext;
	pNew->pPrev = pPrev;
	pPrev->pNext = pNew;
}

inline void LListAddH(LList *pNew, LList *pHead) {

	LListAdd(pNew, pHead, pHead->pNext);
}

inline void LListAddT(LList *pNew, LList *pHead) {

	LListAdd(pNew, pHead->pPrev, pHead);
}

inline void LListDel(LList *pItem) {
	LList *pPrev = pItem->pPrev, *pNext = pItem->pNext;

	pNext->pPrev = pPrev;
	pPrev->pNext = pNext;
	LListInit(pItem);
}

inline LList *LListFirst(LList *pHead) {

	return pHead->pNext != pHead ? pHead->pNext: NULL;
}

inline LList *LListLast(LList *pHead) {

	return pHead->pPrev != pHead ? pHead->pPrev: NULL;
}

inline LList *LListNext(LList *pHead, LList *pItem) {

	return pItem->pNext != pHead ? pItem->pNext: NULL;
}

inline LList *LListPrev(LList *pHead, LList *pItem) {

	return pItem->pPrev != pHead ? pItem->pPrev: NULL;
}


#endif

