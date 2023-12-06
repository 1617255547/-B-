#include"Book.h"
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

//初始化B树
Status InitBTree(BTree& bt) {/*初始化要做全面，避免操作报错，特别是野指针*/
	DestroyBTree(bt);
	bt = (BTree)malloc(sizeof(BTNode));
	bt->Keynum = 0;
	bt->parent = NULL;
	bt->ptr[0] = NULL;
	for (int i = 1; i < m; i++) {
		bt->key[i] = inf;
		bt->book[i] = NULL;
		bt->ptr[i] = NULL;
	}
	return OK;
}

//在结点p中查找关键字k的插入位置i
int SearchBTNode(BTNode* p, KeyType k) {
	if (p == NULL) {
		printf("输入结点为空，查找错误位置\n");
		return 0;
	}
	int i = 1;
	while (i <= p->Keynum && k > p->key[i])
		i++;
	return i;
}

/*在树t上查找关键字k,返回结果(pt,i,tag)。若查找成功,则特征值
 tag=1,关键字k是指针pt所指结点中第i个关键字；否则特征值tag=0,
关键字k的插入位置为pt结点的第i个*/
result SearchBTree(BTree bt, KeyType k) {

	int Tag = 0, i = 0;
	BTree p = bt, q = NULL;
	result r;
	if (bt == NULL) {
		printf("该结点为空,查找错误\n");
		return r;
	}
	while (Tag == 0 && p) {
		i = SearchBTNode(p, k);//找到k在p结点的位置
		if (p->key[i] == k)//如果是k则标1
			Tag = 1;
		else {
			q = p;
			p = p->ptr[i - 1];/*否则进入孩子结点查找*/
		}
	}
	if (Tag) {
		r.pt = p;
		r.i = i;
		r.tag = 1;
	}
	else {//没找到则返回插入的位置的前一个
		r.pt = q;
		r.i = i;
		r.tag = 0;
	}
	return r;
}

/*将关键字k和结点q以及书本信息book分别插入到p->key[i-1]和p->ptr[i]和p->book[i]中*/
void InsertBTNode(BTNode*& p, int i, KeyType k, BTNode* q, BKNode* book) {
	if (p == NULL) {
		printf("该结点为空，无法插入\n");
		return;
	}
	int j;
	for (j = p->Keynum; j >= i; j--) {//i+1后所有元素往后移
		p->key[j + 1] = p->key[j];
		p->book[j + 1] = p->book[j];
		p->ptr[j + 1] = p->ptr[j];
	}
	p->key[i] = k;
	p->book[i] = book;
	p->ptr[i] = q;
	p->Keynum++;//关键字数+1
	if (q != NULL)q->parent = p;//修改父母结点的值
	return;
}

//将p结点分裂成两个结点，前一半保留，后一半移入结点q
void SplitBTNode(BTNode*& p, int s, BTNode*& q) {
	if (p == NULL) {
		printf("p为空节点，无法分裂，已退出\n");
		return;
	}
	int i;
	if (q == NULL)
		BTree q;
	q = (BTree)malloc(sizeof(BTNode));//开空间
	InitBTree(q);
	q->ptr[0] = p->ptr[s];//为了一致性先赋值
	for (i = s + 1; i <= p->Keynum; i++) {//后半部分放进q
		q->key[i - s] = p->key[i];
		q->book[i - s] = p->book[i];
		q->ptr[i - s] = p->ptr[i];
	}
	q->Keynum = p->Keynum - s;//修改关键字数
	q->parent = p->parent;
	for (i = 0; i <= q->Keynum; i++) {/*修改后半部分的父母值*/
		if (q->ptr[i]!=NULL)
			q->ptr[i]->parent = q;
	}

	p->Keynum = s - 1;
	return;
}

//生成新的结点bt，原结点p和q为子树指针
void NewRoot(BTNode*& bt, KeyType k, BTNode* p, BTNode* q, BKNode* book) {
	if (bt == NULL)
		BTree bt = NULL;
	bt = (BTree)malloc(sizeof(BTNode));
	InitBTree(bt);
	bt->key[1] = k;
	bt->book[1] = book;
	bt->Keynum = 1;
	bt->parent = NULL;
	bt->ptr[0] = p;
	bt->ptr[1] = q;
	if (p)p->parent = bt;
	if (q)q->parent = bt;
	return;
}

/*在树bt的结点p的key[i]和key[i+1]插入k，若引起结点过大，则沿双亲分裂*/
void InsertBTree(BTree& bt, int i, KeyType k, BTree p, BKNode* book) {
	if (bt == NULL) {
		printf("头结点为空，插入错误\n");
		return;
	}
	int finish_tag, newroot_tag, s;//设立分裂和新设头结点标志
	BTree q = NULL;//分裂的指针
	KeyType x;//存分裂上移的关键字k
	BKNode* Book;
	if (p == NULL) {//如果插入的结点是空，则新设后退出
		NewRoot(bt, k, NULL, NULL, book);
		return;
	}
	x = k;
	Book = book;
	finish_tag = 0;
	newroot_tag = 0;
	while (!finish_tag && !newroot_tag) {//没完成则继续
		InsertBTNode(p, i, x, q, Book);//插入k
		if (p->Keynum > max) {//大于最大值则分裂
			s = (m + 1) / 2;//前半原置，后半新设，中间上移
			SplitBTNode(p, s, q);//分裂
			x = p->key[s];//存上移关键字
			Book = p->book[s];
			if (p->parent != NULL) {//查找上移的位序
				p = p->parent;
				i = SearchBTNode(p, x);
			}
			else newroot_tag = 1;/*如果分裂到头结点，则新设头结点后退出*/
		}
		else finish_tag = 1;
	}
	if (newroot_tag)NewRoot(bt, x, p, q, Book);
	return;
}

//从结点p删除key[i]和它的孩子指针ptr[i]
void Remove(BTNode* p, int i) {
	if (p == NULL)return;
	for (int j = i + 1; j <= p->Keynum; j++) {/*后续元素前移代删除*/
		p->key[j - 1] = p->key[j];
		p->book[j - 1] = p->book[j];
		p->ptr[j - 1] = p->ptr[j];
	}
	p->key[p->Keynum] = inf;//删除防备
	p->book[p->Keynum] = NULL;
	p->ptr[p->Keynum] = NULL;
	p->Keynum--;
	return;
}

/*查找被删关键字p->key[i](在非叶子结点中)的替代叶子结点（右子树值最小的关键字）*/
void Successor(BTNode* p, int i) {
	if (p == NULL) {
		printf("该查找被删结点为空，输入错误\n");
		return;
	}
	BTree q;
	for (q = p->ptr[i]; q->ptr[0] != NULL; q = q->ptr[0]);
	p->key[i] = q->key[1];//复制关键字
	p->book[i] = q->book[1];
	return;
}

/*将双亲结点p的最后一个关键字移入右结点q，将左结点aq最后一个关键字移入双亲结点p*/
void MoveRight(BTNode* p, int i) {
	if (p == NULL) {
		printf("该结点为空，右移错误，输入错误\n");
		return;
	}
	BTree aq = p->ptr[i - 1], q = p->ptr[i];
	for (int j = q->Keynum; j > 0; j--) {/*将双亲结点p的最后一个关键字移入右结点q，腾出位置*/
		q->key[j + 1] = q->key[j];
		q->book[j + 1] = q->book[j];
		q->ptr[j + 1] = q->ptr[j];
	}
	q->key[1] = p->key[i];
	q->book[1] = p->book[i];
	q->ptr[1] = q->ptr[0];
	p->key[i] = aq->key[aq->Keynum]; /*将左结点aq最后一个关键字移入双亲结点p*/
	p->book[i] = aq->book[aq->Keynum];
	q->ptr[0] = aq->ptr[aq->Keynum];/*最后一个孩子指针给右结点开始*/
	aq->Keynum--;
	q->Keynum++;
	return;
}

/*将双亲结点p的第一个关键字移入左结点aq，将结点q的第一个关键字移入p*/
void MoveLeft(BTNode* p, int i) {
	if (p == NULL) {
		printf("该输入结点为空，左移错误\n");
		return;
	}
	BTree aq = p->ptr[i - 1], q = p->ptr[i];
	aq->Keynum++;
	aq->key[aq->Keynum] = p->key[i];/*将双亲结点p的第一个关键字移入左结点aq*/
	aq->book[aq->Keynum] = p->book[i];
	aq->ptr[aq->Keynum] = q->ptr[0];/*右结点q的第一个孩子放到左结点的最后一个孩子*/
	p->key[i] = q->key[1];//将q第一个放到p
	p->book[i] = q->book[1];
	q->ptr[0] = q->ptr[1];
	Remove(q, 1);//删除q的第一个元素和孩子
	return;
}


//将双亲p、右结点q合并入左结点aq，并调整p剩余关键字的位置
void Combine(BTNode* &p, int i) {
	if (p == NULL) {
		printf("该操作结点为空，合并错误\n");
		return;
	}
	BTree q = p->ptr[i], aq = p->ptr[i - 1];
	int num = aq->Keynum + 1;
	aq->key[num] = p->key[i];//先把p移入
	aq->book[num] = p->book[i];
	aq->ptr[num] = q->ptr[0];
	for (int j = 1; j <= q->Keynum; j++) {//再把右结点全部移入
		num++;
		aq->key[num] = q->key[j];
		aq->book[num] = q->book[j];
		aq->ptr[num] = q->ptr[j];
	}
	aq->Keynum = num;
	Remove(p, i);//删掉p的第i个key
	free(q);//释放空间
	q = NULL;
	if (p->parent != NULL && p->Keynum < min)//检查自身关键字数目是否合规
		Restore(p, i);
	return;
}

//调整B树
void Restore(BTNode* p, int i) {//本是p结点关键字不符合要求
	if (p == NULL) {
		printf("该结点为空，调整错误\n");
		return;
	}
	if (p->parent == NULL)return;
	int j = 0;
	BTree q = p->parent;//先找其父母
	while (q->ptr[j] != p && j <= q->Keynum)
		j++;//找出p是q的第几个孩子
	if (j > q->Keynum) {
		printf("saccaesavaevaevavavavavvawvav");
		return;
	}
	if (j == 0) {/*如果是第一个，则先右借，不够则根节点关键字和右孩子关键字合并到左孩子*/
		if (q->ptr[1]->Keynum > min)
			MoveLeft(q, 1);
		else Combine(q, 1);
	}
	else if (j == q->Keynum) {//最后一个则左借
		if (q->ptr[j - 1]->Keynum > min)
			MoveRight(q, j);
		else Combine(q, j);
	}
	else if (q->ptr[j - 1]->Keynum > min)//在中间则先左后右
		MoveRight(q, j);
	else if (q->ptr[j + 1]->Keynum > min)
		MoveLeft(q, j + 1);
	else Combine(q, j);

	return;
}

//在p查找并删除k
void BTNodeDelete(BTNode* p, KeyType k) {
	if (p == NULL) {
		printf("该结点为空，无法删除\n");
		return;
	}
	int i;
	result r;
	BTree q;
	r = SearchBTree(p, k);//在树p中查找k
	if (r.tag) {
		q = r.pt;
		i = r.i;
		if (q->ptr[i]) {/*非叶子结点删除则在右子树选择最小值替代，然后转化为叶子结点删除*/
			Successor(q, i);
			BTNodeDelete(q->ptr[i], q->key[i]);
			
		}
		else Remove(q, i);//叶子结点先删
		if (q->parent != NULL && q->Keynum < min) {//关键字太少则调整
			Restore(q, i);
		}
	}
	else printf("\n抱歉，该数据已经删除或不存在\n");/*没找到输出返回*/
	return;
}

//删除框架，保护根节点，修bug补全
void BTreeDelete(BTNode*& bt, KeyType k) {
	BTNodeDelete(bt, k);
	if (bt->Keynum == 0 && bt->ptr[0] != NULL) {//合并到头结点则删除头结点，层数-1
		bt = bt->ptr[0];
		bt->parent = NULL;
	}
	return;
}
//递归释放B树
void DestroyBTree(BTree& bt) {
	int i;
	BTree t = bt;
	if (t) {//树不为空就释放
		if (bt->Keynum == 0)return;
		for (i = 0; i <= t->Keynum; i++) {
			DestroyBTree(t->ptr[i]);
		}
		free(t);
	}
	bt = NULL;
	return;
}

//查找节点是否存在
void FindBTNode(BTree& bt, KeyType k) {
	if (bt == NULL) {
		printf("\n该结点为空,查找失败。\n");//空树
		return;
	}
	result r;
	r.i = 0;
	r = SearchBTree(bt, k);
	if (r.tag == 1) {
		printf("\n查找成功。\n");
		return;
	}
	else {
		printf("\n查找失败。\n");
		return;
	}

}

//初始化链表队列 
Status InitQueue(LinkList& L) {
	L = (LinkList)malloc(sizeof(LNode));//分配结点空间
	if (!L)
		return OVERFLOW;//失败
	L->next = NULL;
	return OK;
}

//新建一个结点
LNode* CreateNode(BTNode* p) {
	LinkList q = (LinkList)malloc(sizeof(LNode));/*创建一个链表队列结点*/
	if (q) {
		q->data = p;
		q->next = NULL;
	}
	return q;
}

//元素q入队列
Status Enqueue(LNode* p, BTNode* q) {
	if (!p)return ERROR;
	while (p->next)
		p = p->next;
	p->next = CreateNode(q);//生成结点入队
	return OK;
}
//出队列，并以q返回值
Status Dequeue(LNode* p, BTNode*& q) {
	if (!p && !p->next)return ERROR;//队内无元素
	LNode* l;
	l = p->next;
	p->next = l->next;//修改后续指针域
	q = l->data;//出队
	free(l);//释放
	return OK;
}

//队列判空
Status IfEmpty(LinkList L) {
	if (!L)return ERROR;//
	if (!L->next)return TRUE;
	else return FALSE;
}

//销毁队列
void DestroyQueue(LinkList L) {
	LinkList l;//循环清空释放队列
	while (L) {
		l = L;
		L = L->next;
		free(l);
	}
	return;
}

//用队列遍历输出B树
Status Traverse(BTree bt, LinkList L, int newline, int sum) {
	int i;
	BTree p;
	if (bt) {
		printf("【");
		Enqueue(L, bt->ptr[0]);
		for (i = 1; i <= bt->Keynum; i++) {//入队
			printf(" %d ", bt->key[i]);//先输出
			Enqueue(L, bt->ptr[i]);
		}
		printf("】");
		sum += bt->Keynum + 1;//下一层的孩子数
		if (newline == 0) {//该层孩子已经全部输出，该换行了
			printf("\n");
			newline = sum - 1;//下一层孩子数
			sum = 0;
		}
		else newline--;

	}
	if (IfEmpty(L) == FALSE) {//队列不为空就继续进行出队入队操作
		Dequeue(L, p);
		Traverse(p, L, newline, sum);
	}
	return OK;
}

//输出B树 
Status PrintBTree(BTree bt) {
	if (bt == NULL) {
		printf("\n该树为空树\n");
		return OK;
	}
	LinkList L;
	InitQueue(L);
	Traverse(bt, L, 0, 0);
	DestroyQueue(L);
	return OK;
}

//凹入表形式
void PrintfBTree(BTree bt, int deep) {
	int i, j = deep;
	if (bt != NULL) {
		while (j > 0) {
			printf(" ");
			j--;
		}
		printf("( ");
		for (i = 1; i <= bt->Keynum; i++) {
			printf("%d ", bt->key[i]);
		}
		printf(")\n");
		for (i = 0; i <= bt->Keynum; i++) {
			PrintfBTree(bt->ptr[i], 3 * deep);
		}
	}
	return;
}
//测试B树功能函数
void TestBook() {
	int i = 18, j = 20;
	KeyType k;
	BKNode* book=NULL;
	result r;
	BTree bt = NULL;
	InitBTree(bt);
	int Key[18] = { 35,16,18,70,5,50,22,60,13,17,12,45,25,42,15,90,30,7 };
	/*printf("\n请输入关键字所需个数和最大随机数,空格分开。\n");
	//scanf_s("%d %d", &i, &j);
	//printf("%d,%d\n", i, j);
	//int* Key = (int*)malloc(sizeof(int) * i);
	//srand(time(NULL));
	for (int index = 0; index < i; index++) {
		Key[index] = rand() % j;
		printf("%d ", Key[index]);
	}*/
	printf("\n将创建 %d 阶B树\n", m);
	for (j = 0; j < i; j++) {
		r = SearchBTree(bt, Key[j]);
		if (r.tag == 0)
			InsertBTree(bt, r.i, Key[j], r.pt,book);

	}
	printf("\n已经完成插入\n");
	PrintBTree(bt);
	PrintfBTree(bt, 1);
	while (1) {
		printf("\n此时的B树\n");
		PrintBTree(bt);
		printf("\n");
		PrintfBTree(bt, 1);
		printf("\n=============请输入你需要的操作=============\n");
		printf("  1.初始化     2.插入    3.删除    \n");
		printf("  4.清空释放   5.输出B树 6.查找节点 \n");
		printf("  7.退出   \n");
		printf("=========================================\n");
		printf("输入你选择的功能序号:_____\b\b\b");
		scanf_s("%d", &j);
		switch (j) {
		case 1: {
			InitBTree(bt);
			printf("初始化成功\n");
			break;
		}
		case 2: {
			printf("输入所需插入的关键字:_____\b\b\b");
			scanf_s("%d", &k);
			r = SearchBTree(bt, k);
			if (r.tag == 1) {
				printf("\n结点已经存在，请勿多次插入。\n");
				break;
			}
			InsertBTree(bt, r.i, k, r.pt, book);
			printf("插入成功\n");
			break;
		}
		case 3: {
			printf("输入所需删除的关键字:_____\b\b\b");
			scanf_s("%d", &k);
			BTreeDelete(bt, k);
			printf("\n删除成功\n");
			break;
		}
		case 4: {
			int l;
			printf("请确认是否释放该B树，返回请输入‘1’\n");
			scanf_s("%d", &l);
			if (l == 1)break;
			DestroyBTree(bt);
			printf("\n删除释放完成\n");
			break;
		}
		case 5: {
			PrintBTree(bt);
			PrintfBTree(bt, 1);
			break;
		}
		case 6: {
			printf("输出查找的关键字:_____\b\b\b");
			scanf_s("%d", &k);
			FindBTNode(bt, k);
			break;
		}
		case 7: {
			exit(-1);
			break;
		}
		}
	}
	DestroyBTree(bt);
	printf("已退出\n");
	return;
}

//图书馆书管理操作接口
//检查某本书现存量是否为空
int EmptyBook(BTree bt, KeyType k) {
	if (bt == NULL) {
		printf("该库为空\n");
		return 1;
	}
	result r = SearchBTree(bt, k);
	if (r.tag == 0) {
		printf("没找到此书\n");
		return 1;
	}
	else if (r.pt->book[r.i]->now > 0)
		return 0;
	else return 1;
}
//采编入库
void InsertBook(BTree& bt, KeyType k, result r, char* bookname, char* author, int num) {
	if (bt == NULL) {
		BTree bt;
		bt = (BTree)malloc(sizeof(BTNode));
		InitBTree(bt);
	}
	if (r.tag == 1) {
		r.pt->book[r.i]->now += num;
		r.pt->book[r.i]->total += num;
	}
	else {
		BKNode* book = NULL;
		book = (Book)malloc(sizeof(BKNode));
		book->now = book->total = num;
		strcpy_s(book->bookname, bookname);
		strcpy_s(book->author, author);
		book->reader = NULL;
		InsertBTree(bt, r.i, k, r.pt, book);
	}
}
//清楚库存
void DeleteBook(BTree& bt, KeyType k) {
	BTreeDelete(bt, k);
}
//借阅
void BorrowBook(int readerID, char* readername, result r) {
	if (r.tag == 0) {
		printf("库内没有此书，无法借阅\n");
		return;
	}
	BTNode* p = r.pt;
	int i = r.i;
	Reader* reader;
	reader = (Reader*)malloc(sizeof(Reader));
	strcpy_s(reader->readername, readername);
	reader->readerID = readerID;
	reader->time = day;
	reader->next = p->book[i]->reader;
	p->book[i]->reader = reader;
	p->book[i]->now--;
	return;
}
//归还
void ReturnBook(result r, int readerID) {
	if (r.tag == 0) {
		printf("该书不存在于该图库，请确认输入书号是否有误\n");
		return;
	}
	BTree p = r.pt;
	int i = r.i;
	Reader* reader1 = p->book[i]->reader, * reader2;
	reader2 = reader1;
	while (reader1 != NULL && reader1->readerID != readerID) {
		reader2 = reader1;
		reader1 = reader1->next;
	}
	if (reader1 == NULL) {
		printf("没有查询到该借阅记录\n");
		return;
	}
	else {
		reader2->next = reader1->next;
		free(reader1);
		p->book[i]->now++;
		printf("还书成功\n");
	}
	return;
}
//增加某本书数量
void AddBook(BTree& bt, KeyType k, int number) {
	if (bt == NULL) {
		printf("空库，需要采编入库\n");
		return;
	}
	result r = SearchBTree(bt, k);
	if (r.tag == 0) {
		printf("图库没有该编号书本，请先入库\n");
		return;
	}
	else {
		r.pt->book[r.i]->now += number;
		r.pt->book[r.i]->total += number;
	}
	return;
}
//输出某本书本信息(不包括借阅者)
void PrintfBook(BTree bt, KeyType k) {
	if (bt == NULL) {
		printf("图库已空\n");
		return;
	}
	result r = SearchBTree(bt, k);
	if (r.tag == 0) {
		printf("该书不存在于该图库，请确认输入书号是否有误\n");
		return;
	}
	else {
		printf("|-------------------------------------------------------------------------|\n");
		printf("|书号   书名   作者   现存量  总库存量 |\n");
		printf("|-------------------------------------------------------------------------|\n");
		printf("|%d       %s     %s     %d        %d     |\n", r.pt->key[r.i], r.pt->book[r.i]->bookname, r.pt->book[r.i]->author, r.pt->book[r.i]->now, r.pt->book[r.i]->total);
		printf("|-------------------------------------------------------------------------|\n");
	}
}
//包括借阅者
void PrintfBookAll(BTree bt, KeyType k) {
	PrintfBook(bt, k);
	result r = SearchBTree(bt, k);
	Reader* p = r.pt->book[r.i]->reader;
	while (p != NULL) {
		int j = 1;
		printf("|%d.借阅证号：%d, 姓名：%s, 最迟还书时间还剩：%d 天。\n", j, p->readerID, p->readername, p->time);
		printf("|-------------------------------------------------------------------------|\n");
		p = p->next;
		j++;
	}
	return;
}

int main() {
	BTree bt;
	KeyType k;
	int Key[18] = { 35,16,18,70,5,50,22,60,13,17,12,45,25,42,15,90,30,7 };
	bt = NULL;
	int i,num,total;
	char bookname[NUM], author[NUM], readername[NUM];
	memset(bookname, 0, sizeof(bookname)); // 将字符数组全部置为0
	memset(author, 0, sizeof(author)); 
	memset(readername, 0, sizeof(readername));
	InitBTree(bt);
	PrintfBTree(bt,1);
	printf(" 初始化完成!\n");
	result r;
	for (int j = 0; j < 18; j++) {
		 char s[] = "qwe";
		k = Key[j];
		r = SearchBTree(bt, k);
		if (r.tag == 1) {
			printf("此书本已存在\n");
			PrintfBook(bt, k);
			printf("请输入需要添加此书本的数量\n");
			//scanf_s("%d", &num);
			AddBook(bt, k, k);
			printf("添加完成\n");
		}
		else {
			//printf("请按顺序输入此书的书名、作者（用空格隔开）\n");
			//scanf_s("%s%s", bookname, NUM, author, NUM);
			//printf("请输入需要添加此书本的数量\n");
			//scanf_s("%d", &num);
			InsertBook(bt, k, r, s, s, k);
		}
		printf("添加完成!\n");
		PrintfBook(bt, k);
	}
	while (1)
	{
		printf(" |****************************************************|\n");
		printf(" |*****************|  图书管理系统  |*****************|\n");
		printf(" |****************************************************|\n");
		printf(" |     1：采编入库                 2：清除库存        |\n");
		printf(" |     3：借阅图书                 4：归还图书        |\n");
		printf(" |     5：查看某种图书信息         6：输出B树的状态   |\n");
		printf(" |     0：退出                                        |\n");
		printf(" |****************************************************|\n");
		printf(" |                                                    |\n");
		printf(" |****************************************************|\n");
		printf(" |请输入操作的对应编号：                              |\n");
		printf(" |----------------------------------------------------|\n");
		int i;
		scanf_s("%d", &i);
		switch (i)
		{
		case 1:
			printf("请输入书的编号\n");
			scanf_s("%d", &k);
			r=SearchBTree(bt, k);
			if (r.tag == 1) {
				printf("此书本已存在\n");
				PrintfBook(bt, k);
				printf("请输入需要添加此书本的数量\n");
				scanf_s("%d", &num);
				AddBook(bt, k, num);
			}
			else {
				printf("请按顺序输入此书的书名、作者（用空格隔开）\n");
				scanf_s("%s%s", bookname, NUM, author, NUM);
				printf("请输入需要添加此书本的数量\n");
				scanf_s("%d", &num);
				InsertBook(bt, k, r, bookname, author, num);
			}
			printf("添加完成!\n");
			PrintfBook(bt, k);
			break;
		case 2:
			printf("请输入书的编号\n");
			scanf_s("%d", &k);
			r=SearchBTree(bt, k);
			if (r.tag == 0)
				printf("此书本不存在图库中\n");
			else {
				PrintfBook(bt, k);
				printf("提示：清除后不可恢复！请确认是否清除：\n");
				printf("  1：确认清除       0：取消  \n");
				scanf_s("%d", &i);
				if (i == 1) {
					DeleteBook(bt, k);
					printf("清除成功!\n");
				}
				else
					printf("已取消\n");
			}
			break;
		case 3:
			printf("请输入书的编号\n");
			scanf_s("%d", &k);
			r=SearchBTree(bt, k);
			PrintfBook(bt, k);
			if (EmptyBook(r.pt, r.i)==1)
					printf("此书本现库存不足，暂时无法借阅！\n");
			else {
					printf("请确认是否借书：\n");
					printf("1：确认   0：取消  \n");
					scanf_s("%d", &i);
					if (i == 1) {
						printf("请输入（学号）借阅证号：\n");
						int readernumber;
						scanf_s("%d", &readernumber);
						printf("请输入姓名：\n");
						scanf_s("%s", readername, sizeof(readername));
						BorrowBook(readernumber, readername,r);
						PrintfBookAll(bt, k);
						printf("借书成功!\n");
					}
					else
						printf("已取消\n");
				}
			break;
		case 4:
			printf("请输入书的编号\n");
			scanf_s("%d", &k);
			r=SearchBTree(bt, k);
			PrintfBook(bt, k);
			printf("请确认是否还书：\n");
			printf("1：确认   0：取消  \n");
			scanf_s("%d", &i);
			if (i == 1) {
				printf("请输入借阅证号：\n");
				int readernumber;
				scanf_s("%d", &readernumber);
				ReturnBook(r, readernumber);
			}
			else printf("已取消\n");
			break;
		case 5:
			printf("请输入书的编号\n");
			scanf_s("%d", &k);
			r=SearchBTree(bt, k);
			PrintfBookAll(bt, k);
			break;
		case 6:
			PrintBTree(bt);
			break;
		case 0:
			exit(-1);
			break;
		default:
			printf("输入错误，请重新输入!\n");
			break;
		}
	}
	return 0;
}



