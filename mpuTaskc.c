/*
The goal is define memory access permissions so that an application task can 
only access its own data.

Future Goals:
Prevent a task from corrupting another task's stack by stack leak or overflow.
Prevent an unprivilaged task from accessing a peripheral.

The set-up is that the data region of 'Task A' ,is defined with access permission 
'READ_ONLY' in 'User mode or Unprivilaged'.

Another task, 'Task B' in 'User mode' trying to access the 'READ_ONLY' data region
of the 'Task A'. This results in memMgnt fault and thus successfully protecting the 
'Task A' data corruption. 
*/

/* Declaration of stack for the MPU aware task (GCC syntax) */
static portSTACK_TYPE xTaskStack_A[ 128 ] __attribute__((aligned(128*4)));

/*
RTOS is dynamically changing everytime the access permission for stack to read and write.?
What is the MPU Register configurations in that case ?
Where is this stack in the memory map ?
*/

static portSTACK_TYPE xTaskStack_B[ 128 ] __attribute__((aligned(128*4)));

/* Declaration of a protected array. The Access permission to this is Read-Only. 
Writing to it results fault exception*/
char cReadOnlyArray[ 512 ] __attribute__((aligned(512)));

/*
What is the size of this region ?
-512 Bytes
How do I find the starting address of this region ?
-Some where in the SRAM 
What is the access permission associated with the task to this region ?
-READ_ONLY
What are the memory attributes associated with the task to this region ?
_         _  
 \_(- -)_/ 
*/

/* TaskParameters_t structure to define the task - this is the structure passed to the
xTaskCreateRestricted() function. */
static const TaskParameters_t xTaskDefinition_A =
{
 			vTask_A, /* pvTaskCode */
 			"Task A", /* pcName */
 			128, /* usStackDepth in word length */
 			NULL, /* pvParameters */
 			1, /* uxPriority - priority 1, start in User mode. */
 			xTaskStack_A, /* puxStackBuffer - the array to use as the task stack. */
 			
 			/* xRegions - In this case only one of the three user definable regions is actually used.
 			The parameters are used to set the region to read only. */
 			{
 				/* Base address, Length, Parameters */
 				{ cReadOnlyArray, 512, portMPU_REGION_READ_ONLY },
 				{ 0, 0, 0 },
 				{ 0, 0, 0 },
 			}
};

static const TaskParameters_t xTaskDefinition_B = 
{
		vTask_B,"Task B",128,NULL,1,xTaskStack_B,
		//This is demo task with no access permissions defined. 
		{
			{0,0,0},
			{0,0,0},
			{0,0,0}
		}
};

void* vTask_A(){
	char ch;
	//Read access is valid
	ch = cReadOnlyArray[0];
	//Write access is invalid. Results in memMgnt fault exception
	cReadOnlyArray[0] = ch;
}

void* vTask_B(){
	char ch; 
	//Both read and write access results in memMgnt fault exception
	ch =  cReadOnlyArray[0];
	cReadOnlyArray[0]=ch;
}

void main( void )
{
 	/* Create the task defined by xTaskDefinition. NULL is used as the second parameter as a
 	task handle is not required. */
	 xTaskCreateRestricted( &xTaskDefinition_A, NULL );
	 xTaskCreateRestricted( &xTaskDefinition_B, NULL );
 	/* Start the scheduler. */
 	vTaskStartScheduler();
}
