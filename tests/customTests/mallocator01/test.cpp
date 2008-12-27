/*
 *          _     _
 *          \`\ /`/
 *           \ V /
 *           /. .\            Bunny Kernel for MIPS
 *          =\ T /=
 *           / ^ \
 *        {}/\\ //\
 *        __\ " " /__
 *   jgs (____/^\____)
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
/*!
 *   @author Matus Dekanek, Tomas Petrusek, Lubos Slovak, Jan Vesely
 *   @par "SVN Repository"
 *   svn://aiya.ms.mff.cuni.cz/osy0809-depeslve
 *
 *   @version $Id: header.tmpl 41 2008-10-26 18:00:14Z vesely $
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *
 *   @date 2008-2009
 */

/*!
 * @file
 * @brief Short description.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but
 * at least people can understand it.
 */

#include <api.h>
#include "../include/defs.h"
#include <mem/KernelMemoryAllocator.h>
#include <Kernel.h>

void printHeaderState(BasicMemoryAllocator::BlockHeader * header)
{
	printk("result: size = %d \n",header->size());
	if(header->isUsed()){
		printk("    state: used \n");
	}else if(header->isFree()){
		printk("    state: free \n");
	}else if(header->isBorder()){
		printk("    state: border \n");
	}else{
		printk("    state: unknown \n");
	}

}


void printExpectedState(int arraySize,bool free)
{
	if(free)
	{
		printk("ExpectedResult: \n");
		printk("    size should be 4*%d + 4*6 = %d\n",arraySize,arraySize*4+24);
		printk("    state should be free\n");
	}
	else
	{
		printk("ExpectedResult: \n");
		printk("    size: 4*%d + 4*6 = %d\n",arraySize,arraySize*4+24);
		printk("    state: used\n");
	}

}


void
run_test (void)
{
	printk("starting test..\n");
	//FrameAllocator<7>::instance().init(1024*1024,(uintptr_t) & _kernel_end);

	KernelMemoryAllocator myAllocator;
	int count =10;

	printk("texting list..\n");
	SimpleList pokusList;
	if(!pokusList.empty()){
		printk("error: list not empty\n");
	}
	SimpleListItem * foo;
	printk("inserting 10 items\n");
	for(int i = 0; i<count;i++){
		foo = new SimpleListItem;
		pokusList.insert(foo);
	}
	if(pokusList.empty()){
		printk("failure list is empty\n");
	}
	printk("reconnecting 1 \n");
	foo = pokusList.getMainItem()->next;
	if(foo == pokusList.getMainItem()){
		printk("error main item next = main item \n");
	}
	SimpleListItem * prev = foo->prev;
	SimpleListItem * next = foo->next;
	foo->disconnect();
	foo->next = next;
	foo->prev = prev;
	foo->reconnect();
	if(pokusList.getMainItem()->next != foo){
		printk("error: bad reconnect\n");
	}
	printk("reconnecting 2 (copy ctor) \n");

	foo = pokusList.getMainItem()->next;
	SimpleListItem * bar = new SimpleListItem(*foo);
	foo->disconnect();
	bar->reconnect();
	if(pokusList.getMainItem()->next != bar){
		printk("error: bad reconnect\n");
	}
	printk("list size .. ");
	count = 0;
	foo = pokusList.getMainItem()->next;
	while(foo != pokusList.getMainItem()){
		count ++;
		foo = foo->next;
	}
	printk("%d \n",count);

	printk("removing all - should be 10 items\n");
	int i = 0;
	while(!pokusList.empty()){
		pokusList.getMainItem()->next->disconnect();
		printk(" %d \n",i);
		i++;
	}


	printk("basic list test finished \n");

	printk("--------testing allocator--------\n");
	printk("state consts : \n");
	printk("free   %d \n",BasicMemoryAllocator::BlockHeader::FREE);
	printk("used   %d \n",BasicMemoryAllocator::BlockHeader::USED);
	printk("border %d \n",BasicMemoryAllocator::BlockHeader::BORDER);
	printk("sizeof blockHeader = %x \n",sizeof(BasicMemoryAllocator::BlockHeader));
	printk("sizeof blockFooter = %x \n",sizeof(BasicMemoryAllocator::BlockFooter));



	count = 5;
	int * a;
	a = (int*)myAllocator.getMemory(count* sizeof(int));//array of 5 int
	printk("--------first memory allocated..--------\n");
	for(int i =0; i<count ; i++){
		a[i] = i;
		printk("a[%d] = %d \n",i,a[i]);
	}

	BasicMemoryAllocator::BlockHeader * header;
	BasicMemoryAllocator::BlockHeader * header2;
	BasicMemoryAllocator::BlockHeader * header3;
	header = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)a - sizeof(BasicMemoryAllocator::BlockHeader));
	printk("a      = %x \n",a);
	printk("header = %x \n",header);

	printExpectedState(5,false);
	printHeaderState(header);


	printk("--------free-------- \n");
	myAllocator.freeMemory(a);

	printExpectedState(5,true);
	printHeaderState(header);

	printk("--------alloc more (6 int)--------\n");
	printk("this should lead to creating new free block, if debug implementation is used\n");
	printk("otherwise nothing special is done and first(and only one) free block is used \n");
	a = (int*)myAllocator.getMemory((count+1)* sizeof(int));//array of 6 int
	for(int i =0; i<count+1 ; i++){
		printk("a[%d] = %d(undefined) \n",i,a[i]);
		a[i] = i;
		printk("a[%d] = %d (defined)\n",i,a[i]);
	}
	header2 = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)a - sizeof(BasicMemoryAllocator::BlockHeader));

	printk("--------alloc again 5 int-------- \n");
	a = (int*)myAllocator.getMemory(count* sizeof(int));//array of 5 int
	for(int i =0; i<count ; i++){
		printk("a[%d] = %d(undefined) \n",i,a[i]);
		a[i] = i+5;
		printk("a[%d] = %d (defined as i+5)\n",i,a[i]);
	}
	header3 = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)a - sizeof(BasicMemoryAllocator::BlockHeader));
	printk("next message is relevant only with exact 'frame' creating implementation \n");
	if(header3 == header){
		printk("(debug version of allocator): seems to be correctly allocated\n");
	}else{
		printk("(debug version of allocator): this is strange\n");
	}

	printk("header1\n");
	printExpectedState(5,false);
	printHeaderState(header);
	printk("header2\n");
	printExpectedState(6,false);
	printHeaderState(header2);
	printk("header3\n");
	printExpectedState(5,false);
	printHeaderState(header3);

	printk("freeing both\n");

	printk("free header 1\n");
	myAllocator.freeMemory(a);
	printExpectedState(5,true);
	printHeaderState(header);

	printk("free header 2\n");
	a = (int*)((uintptr_t)header2 + sizeof(BasicMemoryAllocator::BlockHeader));
	myAllocator.freeMemory(a);
	printk("next message is relevant only with exact 'frame' creating implementation \n");
	printExpectedState(6,true);
	printHeaderState(header2);

	printk("--------divide test--------\n");
	header = header2 = header3 = NULL;
	int * b;
	int * c;
	//get space for future 3 blocks, each with seven ints: 2 * (sizeof(blockheader)+footer)+3*7*sizeof(int).
	//7 > 6 what is right now largest block
	a = (int*)myAllocator.getMemory(2*(
		sizeof(BasicMemoryAllocator::BlockHeader)+
		sizeof(BasicMemoryAllocator::BlockFooter))+
		3*7*sizeof(int));
	myAllocator.freeMemory(a);
	printk("now testing dividing as such\n");
	a = (int*)myAllocator.getMemory(7*sizeof(int));
	b = (int*)myAllocator.getMemory(7*sizeof(int));
	c = (int*)myAllocator.getMemory(7*sizeof(int));
	header = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)a - sizeof(BasicMemoryAllocator::BlockHeader));
	header2 = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)b - sizeof(BasicMemoryAllocator::BlockHeader));
	header3 = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)c - sizeof(BasicMemoryAllocator::BlockHeader));
	printExpectedState(7,false);
	printHeaderState(header);
	printExpectedState(7,false);
	printHeaderState(header2);
	printExpectedState(7,false);
	printHeaderState(header3);

	if((header==header2)||(header2==header3)||(header3==header))
	{
		printk("ERROR: allocation on the same place\n");
	}
	printk("--------freeing all: join test--------\n");
	myAllocator.freeMemory(a);
	printHeaderState(header);
	myAllocator.freeMemory(c);
	printHeaderState(header3);
	printk("in next step should be all joined\n");
	printExpectedState(33,true);
	myAllocator.freeMemory(b);
	printHeaderState(header);
	printk("--------free all test--------\n");
	a = (int*)myAllocator.getMemory(7*sizeof(int));
	b = (int*)myAllocator.getMemory(7*sizeof(int));
	c = (int*)myAllocator.getMemory(7*sizeof(int));
	header = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)a - sizeof(BasicMemoryAllocator::BlockHeader));
	header2 = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)b - sizeof(BasicMemoryAllocator::BlockHeader));
	header3 = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)c - sizeof(BasicMemoryAllocator::BlockHeader));
	printExpectedState(7,false);
	printHeaderState(header);
	printExpectedState(7,false);
	printHeaderState(header2);
	printExpectedState(7,false);
	printHeaderState(header3);
	printk("freeing all\n");
	myAllocator.freeAll();
	printExpectedState(33,false);
	printHeaderState(header);
	printk("--------Please check, that everything is deallocated after program end--------\n");
	a = (int*)myAllocator.getMemory(7*sizeof(int));
	b = (int*)myAllocator.getMemory(7*sizeof(int));
	c = (int*)myAllocator.getMemory(7*sizeof(int));
	header = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)a - sizeof(BasicMemoryAllocator::BlockHeader));
	header2 = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)b - sizeof(BasicMemoryAllocator::BlockHeader));
	header3 = (BasicMemoryAllocator::BlockHeader*)((uintptr_t)c - sizeof(BasicMemoryAllocator::BlockHeader));
	printExpectedState(7,false);
	printHeaderState(header);
	printExpectedState(7,false);
	printHeaderState(header2);
	printExpectedState(7,false);
	printHeaderState(header3);
	//allocating/deallocating
	//int * pointerArray[10];
	int ** pointerArray = (int**)myAllocator.getMemory(10*sizeof(uintptr_t));
	//alloc
	for(i=1;i<=10;i++)
	{
		pointerArray[i] = (int*)myAllocator.getMemory(i*sizeof(int));
	}
	//dealloc
	for(i=10;i>0;i--)
	{
		myAllocator.freeMemory(pointerArray[i]);
	}
	printk("total free size = %d \n",myAllocator.getFreeSize());
	//alloc/dealoc
	for(i=1;i<=10;i++)
	{
		pointerArray[i] = (int*)myAllocator.getMemory(i*sizeof(int));
		if(i%2 > 0){
			myAllocator.freeMemory(pointerArray[i]);
		}
	}
	//dealloc/alloc
	for(i=1;i<=10;i++)
	{
		if(i%2 > 0)
			pointerArray[i] = (int*)myAllocator.getMemory(i*sizeof(int));
		else
			myAllocator.freeMemory(pointerArray[i]);
	}

	printk("total free size = %d\n",myAllocator.getFreeSize());
	//dealloc rest
	for(i=1;i<=10;i++)
	{
		if(i%2 > 0)
			myAllocator.freeMemory(pointerArray[i]);
	}
	printk("total free size = %d\n",myAllocator.getFreeSize());
	myAllocator.freeAll();
	printk("total free size = %d\n",myAllocator.getFreeSize());



	// print the result
	printk ("Test passed...\n");
}







