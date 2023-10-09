

#ifndef ESKF_3dofcute_HEADER_FLAG

#define ESKF_3dofcute_HEADER_FLAG

#define ESKF_3dofcute_fp_count (16*36)

#define ESKF_3dofcute_fp_double

#include <math.h>
#include <android/log.h>

#ifdef ESKF_3dofcute_fp_double

typedef double ESKF_3dofcute_fp_type;
#define ESKF_3dofcute_sin sin
#define ESKF_3dofcute_cos cos
#define ESKF_3dofcute_acos acos
#define ESKF_3dofcute_asin asin
#define ESKF_3dofcute_tan tan
#define ESKF_3dofcute_atan atan
#define ESKF_3dofcute_sqrt sqrt

#else

typedef float ESKF_3dofcute_fp_type;
#define ESKF_3dofcute_sin sinf
#define ESKF_3dofcute_cos cosf
#define ESKF_3dofcute_acos acosf
#define ESKF_3dofcute_asin asinf
#define ESKF_3dofcute_tan tanf
#define ESKF_3dofcute_atan atanf
#define ESKF_3dofcute_sqrt sqrtf

#endif


//typedef ESKF_3dofcute_fp_type (*ESKF_3dofcute_math)(ESKF_3dofcute_fp_type);
//
//ESKF_3dofcute_math sin;
//ESKF_3dofcute_math cos;
//ESKF_3dofcute_math asin;
//ESKF_3dofcute_math acos;
//ESKF_3dofcute_math tan;
//ESKF_3dofcute_math atan;
//ESKF_3dofcute_math sqrt;

#define TAG "USBProxy-jni--" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型

typedef struct {
    ESKF_3dofcute_fp_type *raw_array;


    ESKF_3dofcute_fp_type *Q_temp;
    ESKF_3dofcute_fp_type *Q_prev;
    ESKF_3dofcute_fp_type *Q_curr;
    ESKF_3dofcute_fp_type *Vg;
    ESKF_3dofcute_fp_type *Pk_prev;
    ESKF_3dofcute_fp_type *Pk_curr;
    ESKF_3dofcute_fp_type *delta_X_prev;
    ESKF_3dofcute_fp_type *delta_X_curr;
    ESKF_3dofcute_fp_type *delta_Xp;
    ESKF_3dofcute_fp_type *buf;
    ESKF_3dofcute_fp_type *buf2;
    ESKF_3dofcute_fp_type *buf3;
    ESKF_3dofcute_fp_type *Qp;
    ESKF_3dofcute_fp_type *Q_delta_X;
    ESKF_3dofcute_fp_type *F_delta_X;
    ESKF_3dofcute_fp_type *Rk;
    ESKF_3dofcute_fp_type *u;
    ESKF_3dofcute_fp_type *S;
    ESKF_3dofcute_fp_type *SS;
    ESKF_3dofcute_fp_type *E3;
    ESKF_3dofcute_fp_type *R1;
    ESKF_3dofcute_fp_type *R2;
    ESKF_3dofcute_fp_type *R3;
    ESKF_3dofcute_fp_type *J1;
    ESKF_3dofcute_fp_type *J2;
    ESKF_3dofcute_fp_type *J3;
    ESKF_3dofcute_fp_type *X_delta_x;
    ESKF_3dofcute_fp_type *H_acc;
    ESKF_3dofcute_fp_type *h_acc;
    ESKF_3dofcute_fp_type *Hk;
    ESKF_3dofcute_fp_type *P_k;
    ESKF_3dofcute_fp_type *Pk2_;
    ESKF_3dofcute_fp_type *Kk;
    ESKF_3dofcute_fp_type *G;
    ESKF_3dofcute_fp_type *delta_X_hat;
    ESKF_3dofcute_fp_type t;
    ESKF_3dofcute_fp_type norm_a;
    ESKF_3dofcute_fp_type norm_g;
} ESKF_3dofcute_status;

#ifdef __cplusplus
extern "C" {
#endif

static ESKF_3dofcute_fp_type Bax = -43.41564541787997;//0;
static ESKF_3dofcute_fp_type Bay = 16.81689991767666;//0;
static ESKF_3dofcute_fp_type Baz = -0.27422300320112;//0;
static ESKF_3dofcute_fp_type Bgx = -10.75146318601207;// 0;
static ESKF_3dofcute_fp_type Bgy = -13.05151844057756;// 0;
static ESKF_3dofcute_fp_type Bgz = -1.91339782051883;//0;
static ESKF_3dofcute_fp_type Kax = 0.00474758190538;// 0.004785156250000;
static ESKF_3dofcute_fp_type Kay = 0.00477623070014;// 0.004785156250000;
static ESKF_3dofcute_fp_type Kaz = 0.00478294211209;// 0.004785156250000;
static ESKF_3dofcute_fp_type Kgx = 0.00126253708078;// 0.001065264436032;
static ESKF_3dofcute_fp_type Kgy = 0.00123420038935;// 0.001065264436032;
static ESKF_3dofcute_fp_type Kgz = 0.00123541017537;// 0.001065264436032;
static ESKF_3dofcute_fp_type table_mean0 = 0;
static ESKF_3dofcute_fp_type table_mean1 = 0;
static ESKF_3dofcute_fp_type table_mean2 = 0;
static ESKF_3dofcute_fp_type table_mean3 = 0;
static ESKF_3dofcute_fp_type table_mean4 = 0;
static int n = 0;
static int temp_n = -1;
static int nn = 0;
static int flag = 0;
static int flagmean = 0;
static ESKF_3dofcute_fp_type table_gyro_raw0[401];
static ESKF_3dofcute_fp_type table_gyro_raw1[401];
static ESKF_3dofcute_fp_type table_gyro_raw2[401];
static ESKF_3dofcute_fp_type table_gyro_raw3[401];
static ESKF_3dofcute_fp_type table_gyro_raw4[401];
static ESKF_3dofcute_fp_type wx1 = 0;
static ESKF_3dofcute_fp_type wx2 = 0;
static ESKF_3dofcute_fp_type wy1 = 0;
static ESKF_3dofcute_fp_type wy2 = 0;
static ESKF_3dofcute_fp_type wz1 = 0;
static ESKF_3dofcute_fp_type wz2 = 0;
static int n_win = 200;
//int n_win2_1 = n_win * 2 + 1;   // 401
static int n_win2_1 = 401;   // 401
static ESKF_3dofcute_fp_type a[2] = {1, -0.984414127416097};
static ESKF_3dofcute_fp_type b[2] = {0.007792936291952, 0.007792936291952};

void ESKF_3dofcute_status_new(ESKF_3dofcute_status **const ptr);

void ESKF_3dofcute_status_delete(ESKF_3dofcute_status **const ptr);
static ESKF_3dofcute_fp_type init_angle_threshold = 0.867;
void ESKF_3dofcute_status_init(
        ESKF_3dofcute_fp_type *Q_result,
        ESKF_3dofcute_status *const ptr,
        ESKF_3dofcute_fp_type const acc_x,
        ESKF_3dofcute_fp_type const acc_y,
        ESKF_3dofcute_fp_type const acc_z
);

void ESKF_3dofcute_step(
        ESKF_3dofcute_fp_type *Q_result,
        ESKF_3dofcute_status *const status,
        ESKF_3dofcute_fp_type const acc_x,
        ESKF_3dofcute_fp_type const acc_y,
        ESKF_3dofcute_fp_type const acc_z,
        ESKF_3dofcute_fp_type const gyro_x,
        ESKF_3dofcute_fp_type const gyro_y,
        ESKF_3dofcute_fp_type const gyro_z
);

void calculateAccAndGyro(
        ESKF_3dofcute_fp_type *ACC_result, ESKF_3dofcute_fp_type *GYRO_result,
        ESKF_3dofcute_fp_type const acc_x,
        ESKF_3dofcute_fp_type const acc_y,
        ESKF_3dofcute_fp_type const acc_z,
        ESKF_3dofcute_fp_type const gyro_x,
        ESKF_3dofcute_fp_type const gyro_y,
        ESKF_3dofcute_fp_type const gyro_z);

void computeBgOnlineMean(
        ESKF_3dofcute_fp_type gyro_raw_x, ESKF_3dofcute_fp_type gyro_raw_y,
        ESKF_3dofcute_fp_type gyro_raw_z);

void initIntrinsics(
        ESKF_3dofcute_fp_type const Bax_param, ESKF_3dofcute_fp_type const Bay_param,
        ESKF_3dofcute_fp_type const Baz_param,
        ESKF_3dofcute_fp_type const Kax_param, ESKF_3dofcute_fp_type const Kay_param,
        ESKF_3dofcute_fp_type const Kaz_param,
        ESKF_3dofcute_fp_type const Bgx_param, ESKF_3dofcute_fp_type const Bgy_param,
        ESKF_3dofcute_fp_type const Bgz_param,
        ESKF_3dofcute_fp_type const Kgx_param, ESKF_3dofcute_fp_type const Kgy_param,
        ESKF_3dofcute_fp_type const Kgz_param);

void getIntrinsics(double accScale[], double accOffset[], double gyroScale[], double gyroOffset[]);

void get_Screen_Attitude(
        ESKF_3dofcute_fp_type*  Qcsreen,
        ESKF_3dofcute_fp_type*  Pcsreen,
        ESKF_3dofcute_fp_type*   q
);

void changePscreen2Unity(
        ESKF_3dofcute_fp_type* PscreenUnity,
        ESKF_3dofcute_fp_type* Pscreen
);

void changeQscreen2Unity(
        ESKF_3dofcute_fp_type* QscreenUnity,
        ESKF_3dofcute_fp_type* Qscreen
);

#ifdef __cplusplus
}
#endif

#endif