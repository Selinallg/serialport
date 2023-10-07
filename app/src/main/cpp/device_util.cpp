//
// Created by Administrator on 2022/9/8.
//

#include "device_util.h"
#include <chrono>

int device_util::get_thread_policy( pthread_attr_t &attr )
{
    int policy;
    int rs = pthread_attr_getschedpolicy( &attr, &policy );
    assert( rs == 0 );
    switch ( policy )
    {
        case SCHED_FIFO:
            //kInfo("SCHED_FIFO");
            break;
        case SCHED_RR:
            //kInfo("SCHED_RR");
            break;
        case SCHED_OTHER:
            //kInfo("SCHED_OTHER");
            break;
        default:
            //kInfo("unknown");
            break;
    }
    return policy;
}

void device_util::show_thread_priority( pthread_attr_t &attr, int policy )
{
    int priority = sched_get_priority_max( policy );
    //kInfo("max_priority: {} ", priority);
    priority = sched_get_priority_min( policy );
    //kInfo("min_priority: {} ", priority);
}

int device_util::get_thread_priority( pthread_attr_t &attr )
{
    struct sched_param param{};
    int rs = pthread_attr_getschedparam( &attr, &param );
    //kInfo("priority = {}", param.sched_priority);
    return param.sched_priority;
}

int device_util::set_thread_priority( pthread_attr_t &attr )
{
    struct sched_param param{};
    param.sched_priority  = 99;
    int rs = pthread_attr_setschedparam( &attr, &param );
    return param.sched_priority;
}

void device_util::set_thread_policy( pthread_attr_t &attr,  int policy )
{
    int rs = pthread_attr_setschedpolicy( &attr, policy );
    get_thread_policy( attr );
}


void device_util::set_cur_thread_affinity(int id) {
//    auto cors = sysconf(_SC_NPROCESSORS_CONF);
    auto cpu_id = id;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);

    pid_t pid = gettid();
    auto ret = syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);
    if (ret) {
        //kInfo("syscallres error: {}", errno);
    }
    //kInfo("set thread affinity success");
}


void device_util::set_thread_priority_self(){
    pid_t pid =getpid();

    int curschdu = sched_getscheduler(pid);
    if(curschdu <0 )
    {
        //kError("getschedu err: {}", errno);
    }
    //kInfo("use tid schedu befor: {}", curschdu);

    struct sched_param s_parm{};
    s_parm.sched_priority = 0;//sched_get_priority_max(SCHED_FIFO);//低于高通qvr线程优先级
    //kInfo("schedu max {} min {}", sched_get_priority_max(SCHED_FIFO),sched_get_priority_min(SCHED_FIFO));

    auto ret = sched_setscheduler(pid, SCHED_OTHER, &s_parm);
    if(ret <0)
    {
        //kWarn("setschedu err: {}", strerror(errno));
    }

    curschdu = sched_getscheduler(pid);

    //kInfo("schedu after: {}", curschdu);
}

void device_util::set_self_thread_priority(){
    pthread_attr_t attr;
    struct sched_param sched{};
    int rs;
    rs = pthread_attr_init( &attr );
    if(rs != 0){
        //kError("pthread_attr_init error: {}", rs);
        return;
    }

    int policy = get_thread_policy( attr );
    //kInfo("Show current configuration of priority");
    show_thread_priority( attr, policy );
    //kInfo("Show SCHED_FIFO of priority");
    show_thread_priority( attr, SCHED_FIFO );

    //kInfo("Show SCHED_RR of priority");
    show_thread_priority( attr, SCHED_RR );

    //kInfo("Show priority of current thread");
    set_thread_priority( attr );
    get_thread_priority( attr );

    //kInfo("Set SCHED_FIFO policy");
    set_thread_policy( attr, SCHED_FIFO );
//    //kInfo("Set SCHED_RR policy");
//    set_thread_policy( attr, SCHED_RR );
//    //kInfo("Restore current policy");
//    set_thread_policy( attr, policy );

    rs = pthread_attr_destroy( &attr );
    if(rs != 0 ){
        //kWarn("pthread_attr_destroy error");
    }
}


int32_t device_util::SystemProp_get(const char *key, int32_t default_value) {
    int len;
    char buf[PROP_VALUE_MAX];
    char *end;
    int32_t result = default_value;

    len = __system_property_get(key, buf);
    if (len > 0) {
        result = strtol(buf, &end, 0);
    }
    return result;
}

void device_util::SystemProp_set(const char *key, int32_t value)
{
    __system_property_set(key, std::to_string(value).c_str());
}

pid_t device_util::getProcessPidByName(const char *proc_name)
{
    FILE *fp;
    char buf[100];
    char cmd[200] = {'\0'};
    pid_t pid = -1;
//    sprintf(cmd,"pidof %s", proc_name);
//    if((fp = popen(cmd, "r")) != NULL)
//    {
//        if(fgets(buf,255, fp) != NULL){
//            pid = atoi(buf);
//        }
//    }
//    //  printf("pid = %d \n", pid);

    pclose(fp);
    return pid;
}




void device_util::set_affinity_mask(uint16_t affinity_mask)
{
    int rc ;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    cpu_set_t cpu_array;
    CPU_ZERO(&cpu_array);

    affinity_mask=affinity_mask&0x00ff;
    for(int i=0; i<8;i++) {
        uint16_t flag=(affinity_mask>>i)&0x01;
        if(flag)
            CPU_SET(i,&cpu_array);
    }

    cpu_set_t cpuget;
    CPU_ZERO(&cpuget);

    cpu_set_t cpu_curr;
    CPU_ZERO(&cpu_curr);
    rc=sched_getaffinity(0, sizeof(cpu_curr), &cpu_curr);
    assert (rc == 0);

    CPU_AND(&cpuset,&cpu_curr,&cpu_array);//将当前可亲核cpu 集与期望的亲核集做逻辑与操作

    sched_setaffinity(0,sizeof(cpu_set_t), &cpuset);
}

int64_t device_util::get_timestamp_ms() {
    return std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

float device_util::SystemProp_get_float(const char *key, float default_value) {
    int len;
    char buf[PROP_VALUE_MAX];
    char *end = 0;
    float result = default_value;

    len = __system_property_get(key, buf);
    if (len > 0) {
        result = std::strtof(buf, &end);
    }
    return result;
}

void device_util::SystemProp_set_float(const char *key, float value) {
    __system_property_set(key, std::to_string(value).c_str());
}
