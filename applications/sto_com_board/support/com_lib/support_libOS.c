/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : vcp_test_main.c
 **@author: Created By Chengt
 **@date  : 2022.02.21
 **@brief : Implement the function interfaces of vcp_test_main
 ********************************************************************************************/
#include "if_os.h"
#include "if_app.h"
#include "support_libOS.h"

/*******************************************************************************************
 *        Local definitions
 *******************************************************************************************/
#define OS_SEM_DISABLE

#define TASK_TABLE_SIZE   ( 100U )
#define EVENT_TABLE_SIZE  ( 100U )

/* ����������Դ */
typedef struct
{
    uint8 table_used_size;
    task_func task_table[TASK_TABLE_SIZE];
} S_TASK_TABLE_INFO;

/* �����ź�����Դ */
typedef struct
{
    uint8 sem_table_size;
    OS_EVENT g_EVENT_TABLE[EVENT_TABLE_SIZE];
} S_SEM_TABLE_INFO;

/*******************************************************************************************
 *        Local variables
 *******************************************************************************************/
/* �����ϲ�������Դ�� */
static S_SEM_TABLE_INFO g_Sem_Table;

/* �����ϲ��ź�����Դ�� */
static S_TASK_TABLE_INFO g_Task_Table[4U];

/* �ϲ�Ӧ�ó�ʼ���ӿ� */
static task_func g_app_init = NULL;

/*******************************************************************************************
 *        Local functions
 *******************************************************************************************/
/*******************************************************************************************
 ** @brief: task_proc_01
 ** @param: null
 *******************************************************************************************/
static void task_proc_01(void)
{
    uint8 i = 0U;

    for (i = 0U; i <= g_Task_Table[TASK01_ID].table_used_size; i++)
    {
        if (NULL != g_Task_Table[TASK01_ID].task_table[i])
        {
            g_Task_Table[TASK01_ID].task_table[i]();
        }
    }
}

/*******************************************************************************************
 ** @brief: task_proc_02
 ** @param: null
 *******************************************************************************************/
static void task_proc_02(void)
{
    uint8 i = 0U;

    for (i = 0U; i <= g_Task_Table[TASK02_ID].table_used_size; i++)
    {
        if (NULL != g_Task_Table[TASK02_ID].task_table[i])
        {
            g_Task_Table[TASK02_ID].task_table[i]();
        }
    }
}

/*******************************************************************************************
 ** @brief: task_proc_03
 ** @param: null
 *******************************************************************************************/
static void task_proc_03(void)
{
    uint8 i = 0U;

    for (i = 0U; i <= g_Task_Table[TASK03_ID].table_used_size; i++)
    {
        if (NULL != g_Task_Table[TASK03_ID].task_table[i])
        {
            g_Task_Table[TASK03_ID].task_table[i]();
        }
    }
}

/*******************************************************************************************
 ** @brief: task_proc_04
 ** @param: null
 *******************************************************************************************/
static void task_proc_04(void)
{
    uint8 i = 0U;

    for (i = 0; i <= g_Task_Table[TASK04_ID].table_used_size; i++)
    {
        if (NULL != g_Task_Table[TASK04_ID].task_table[i])
        {
            g_Task_Table[TASK04_ID].task_table[i]();
        }
    }
}

/*******************************************************************************************
 ** @brief: task_proc_04
 ** @param: null
 *******************************************************************************************/
static void app_Init(void)
{
    /* 1.ִ���ϲ��ʼ�� */
    if (NULL != g_app_init)
    {
        g_app_init();
    }
}

/*******************************************************************************************
 ** @brief: app_polling_proc
 ** @param: null
 *******************************************************************************************/
static void app_polling_proc(void)
{
    /* 1.����1���� */
    task_proc_01();

    /* 2.����2���� */
    task_proc_02();

    /* 3.����3���� */
    task_proc_03();
}

/*******************************************************************************************
 ** @brief: app_idle_proc
 ** @param: null
 *******************************************************************************************/
static void app_idle_proc(void)
{
    /* 1.����4���� */
    task_proc_04();
}

/*******************************************************************************************
 ** @brief: appArchIf_init
 ** @param: null
 *******************************************************************************************/
static void appArchIf_init(void)
{
    /* 1.Ӧ�ûص��ӿڳ�ʼ�� */
    g_appArchIf.app_init = app_Init;
    g_appArchIf.app_polling_proc = app_polling_proc;
    g_appArchIf.app_idle_proc = app_idle_proc;
}
/*******************************************************************************************
 ** @brief: support_osRunning
 ** @param: p_init
 *******************************************************************************************/
extern void support_osRunning(task_func p_init)
{
    g_app_init = p_init;

    app_archRunning(appArchIf_init);
}

/*******************************************************************************************
 ** @brief: support_osRegisterFunc
 ** @param: task_id, p_func
 *******************************************************************************************/
extern BOOL support_osRegisterFunc(E_SYS_TASK_ID task_id, task_func p_func)
{
    uint8 used_size = 0U;
    used_size = g_Task_Table[task_id].table_used_size;
    if (used_size < ( TASK_TABLE_SIZE))
    {
        g_Task_Table[task_id].task_table[used_size] = p_func;
        g_Task_Table[task_id].table_used_size++;
        MY_Printf("priority %d  task %d  register func ok !!! \r\n ", task_id, used_size);
        return TRUE;
    }
    else
    {
        MY_Printf(" task table is full !!! \r\n ");
        return FALSE;
    }
}

/*******************************************************************************************
 ** @brief: support_SemCreate
 ** @param: sem_id
 *******************************************************************************************/
extern BOOL support_SemCreate(uint8 *sem_id)
{
    uint8 size = 0;
    size = g_Sem_Table.sem_table_size;
    BOOL retFlg = FALSE;

#ifdef OS_SEM_DISABLE
    return TRUE;
#endif

    if (size < EVENT_TABLE_SIZE)
    {
        *sem_id = size;
        if ( OK == if_OSSemCreate(&g_Sem_Table.g_EVENT_TABLE[size]))
        {
            g_Sem_Table.sem_table_size++;
            retFlg = TRUE;
        }
        else
        {
            retFlg = FALSE;
        }
    }
    else
    {
        retFlg = FALSE;
    }

    return retFlg;
}

/*******************************************************************************************
 ** @brief: support_SemPend
 ** @param: sem_id
 *******************************************************************************************/
extern BOOL support_SemPend(uint8 sem_id)
{
    sint8 err;

#ifdef OS_SEM_DISABLE
    return TRUE;
#endif

    if_OSSemPend(&g_Sem_Table.g_EVENT_TABLE[sem_id], 0, &err);
    return TRUE;
}

/*******************************************************************************************
 ** @brief: support_SemPost
 ** @param: sem_id
 *******************************************************************************************/
extern BOOL support_SemPost(uint8 sem_id)
{
#ifdef OS_SEM_DISABLE
    return TRUE;
#endif

    if_OSSemPost(&g_Sem_Table.g_EVENT_TABLE[sem_id]);
    return TRUE;
}

/**************************************end file*********************************************/
