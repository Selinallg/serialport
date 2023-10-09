#define ESKF_3dofcute_DEMO

#ifdef ESKF_3dofcute_DEMO
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#endif

#include "ESKF_3dofcute.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

//#define M_PI 3.141592654



static ESKF_3dofcute_fp_type norm(
        ESKF_3dofcute_fp_type *const ptr,
        int const size
) {
    int i;
    ESKF_3dofcute_fp_type sum;
    sum = 0;
    for (i = 0; i < size; ++i) {
        sum += ptr[i] * ptr[i];
    }
    return ESKF_3dofcute_sqrt(sum);
}

static void eye(
        ESKF_3dofcute_fp_type *const ptr,
        int const size,
        ESKF_3dofcute_fp_type const val
) {
    int i, j;
    int p;
    p = 0;
    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            if (i != j) {
                ptr[p] = 0;
            } else {
                ptr[p] = val;
            }
            ++p;
        }
    }
}

static void mat_mul(
        ESKF_3dofcute_fp_type *const ret,
        ESKF_3dofcute_fp_type *const left,
        ESKF_3dofcute_fp_type *const right,
        int const ret_row,
        int const size,
        int const ret_col
) {
    int i, j, k;
    ESKF_3dofcute_fp_type sum;
    int p;
    p = 0;
    for (i = 0; i < ret_row; ++i) {
        for (j = 0; j < ret_col; ++j) {
            sum = 0;
            for (k = 0; k < size; ++k) {
                sum += left[size * i + k] * right[ret_col * k + j];
            }
            ret[p] = sum;
            ++p;
        }
    }
}

static void mat_add(
        ESKF_3dofcute_fp_type *const ret,
        ESKF_3dofcute_fp_type *const left,
        ESKF_3dofcute_fp_type *const right,
        int const ret_row,
        int const ret_col,
        ESKF_3dofcute_fp_type const xleft,
        ESKF_3dofcute_fp_type const xright
) {
    int i, j;
    int p;
    p = 0;
    for (i = 0; i < ret_row; ++i) {
        for (j = 0; j < ret_col; ++j) {
            ret[p] = xleft * left[p] + xright * right[p];
            ++p;
        }
    }
}

static void mat_t(
        ESKF_3dofcute_fp_type *const ret,
        ESKF_3dofcute_fp_type *const orig,
        int const ret_row,
        int const ret_col
) {
    int i, j;
    int p;
    p = 0;
    for (i = 0; i < ret_row; ++i) {
        for (j = 0; j < ret_col; ++j) {
            ret[p] = orig[ret_row * j + i];
            ++p;
        }
    }
}

static void mat_inv3(
        ESKF_3dofcute_fp_type *const ret,
        ESKF_3dofcute_fp_type *const orig
) {
    ESKF_3dofcute_fp_type n;
    n = orig[0 * 3 + 0] * (orig[1 * 3 + 1] * orig[2 * 3 + 2] - orig[1 * 3 + 2] * orig[2 * 3 + 1])
        - orig[1 * 3 + 0] * (orig[0 * 3 + 1] * orig[2 * 3 + 2] - orig[0 * 3 + 2] * orig[2 * 3 + 1])
        + orig[2 * 3 + 0] * (orig[0 * 3 + 1] * orig[1 * 3 + 2] - orig[0 * 3 + 2] * orig[1 * 3 + 1]);

    ret[0 * 3 + 0] = (orig[1 * 3 + 1] * orig[2 * 3 + 2] - orig[1 * 3 + 2] * orig[2 * 3 + 1]) / n;
    ret[1 * 3 + 0] = -(orig[1 * 3 + 0] * orig[2 * 3 + 2] - orig[1 * 3 + 2] * orig[2 * 3 + 0]) / n;
    ret[2 * 3 + 0] = (orig[1 * 3 + 0] * orig[2 * 3 + 1] - orig[1 * 3 + 1] * orig[2 * 3 + 0]) / n;
    ret[0 * 3 + 1] = -(orig[0 * 3 + 1] * orig[2 * 3 + 2] - orig[0 * 3 + 2] * orig[2 * 3 + 1]) / n;
    ret[1 * 3 + 1] = (orig[0 * 3 + 0] * orig[2 * 3 + 2] - orig[0 * 3 + 2] * orig[2 * 3 + 0]) / n;
    ret[2 * 3 + 1] = -(orig[0 * 3 + 0] * orig[2 * 3 + 1] - orig[0 * 3 + 1] * orig[2 * 3 + 0]) / n;
    ret[0 * 3 + 2] = (orig[0 * 3 + 1] * orig[1 * 3 + 2] - orig[0 * 3 + 2] * orig[1 * 3 + 1]) / n;
    ret[1 * 3 + 2] = -(orig[0 * 3 + 0] * orig[1 * 3 + 2] - orig[0 * 3 + 2] * orig[1 * 3 + 0]) / n;
    ret[2 * 3 + 2] = (orig[0 * 3 + 0] * orig[1 * 3 + 1] - orig[0 * 3 + 1] * orig[1 * 3 + 0]) / n;
}

static void Skew_symmetric(
        ESKF_3dofcute_fp_type *const ret,
        ESKF_3dofcute_fp_type *const u
) {
    ret[0] = 0;
    ret[1] = -u[2];
    ret[2] = u[1];
    ret[3] = u[2];
    ret[4] = 0;
    ret[5] = -u[0];
    ret[6] = -u[1];
    ret[7] = u[0];
    ret[8] = 0;
}

static void quatern2rotMat(
        ESKF_3dofcute_fp_type *const R,
        ESKF_3dofcute_fp_type *const q
) {
    ESKF_3dofcute_fp_type w, x, y, z;
    w = q[0];
    x = q[1];
    y = q[2];
    z = q[3];

    R[0] = w * w + x * x - y * y - z * z;
    R[1] = 2 * (x * y - w * z);
    R[2] = 2 * (x * z + w * y);
    R[3] = 2 * (x * y + w * z);
    R[4] = w * w + y * y - x * x - z * z;
    R[5] = 2 * (y * z - w * x);
    R[6] = 2 * (x * z - w * y);
    R[7] = 2 * (y * z + w * x);
    R[8] = w * w + z * z - x * x - y * y;
}

static void quatern2axisAngle(
        ESKF_3dofcute_fp_type *const axis,
        ESKF_3dofcute_fp_type *const angle,
        ESKF_3dofcute_fp_type *const q
) {
    ESKF_3dofcute_fp_type t;
    ESKF_3dofcute_fp_type len;

    len = norm(q, 4);
    if (q[0] < 0) {
        len = -len;
    }
    t = q[0] / len;
    if (t > 1) {
        t = 1;
    }
    t = ESKF_3dofcute_acos(t) * 2;
    *angle = t;
    t = ESKF_3dofcute_sin(t / 2) * len;
    if (t) {
        axis[0] = q[1] / t;
        axis[1] = q[2] / t;
        axis[2] = q[3] / t;
    } else {
        axis[0] = 0;
        axis[1] = 0;
        axis[2] = 0;
    }
}

static void axisAngle2quatern(
        ESKF_3dofcute_fp_type *const q,
        ESKF_3dofcute_fp_type *const axis,
        ESKF_3dofcute_fp_type *const angle
) {
    ESKF_3dofcute_fp_type t;
    t = *angle / 2;
    q[0] = ESKF_3dofcute_cos(t);
    t = ESKF_3dofcute_sin(t);
    q[1] = axis[0] * t;
    q[2] = axis[1] * t;
    q[3] = axis[2] * t;
}

static void cross3(
        ESKF_3dofcute_fp_type *const V3,
        ESKF_3dofcute_fp_type *const V1,
        ESKF_3dofcute_fp_type *const V2
) {
    V3[0] = V1[1] * V2[2] - V1[2] * V2[1];
    V3[1] = V1[2] * V2[0] - V2[2] * V1[0];
    V3[2] = V1[0] * V2[1] - V2[0] * V1[1];
}

static void quaternProd(
        ESKF_3dofcute_fp_type *const ab,
        ESKF_3dofcute_fp_type *const a,
        ESKF_3dofcute_fp_type *const b
) {
    //ESKF_3dofcute_fp_type len;
    ab[0] = a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3];
    ab[1] = a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2];
    ab[2] = a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1];
    ab[3] = a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0];

}

static void qUtoV(
        ESKF_3dofcute_fp_type *const q,
        ESKF_3dofcute_fp_type *const v1,
        int v2d
) {
    ESKF_3dofcute_fp_type v[4];
    v[1] = v1[0];
    v[2] = v1[1];
    v[3] = v1[2];
    v[0] = norm(v + 1, 3);
    v[1] /= v[0];
    v[2] /= v[0];
    v[3] /= v[0];
    v[v2d + 1] += 1;
    v[0] = norm(v + 1, 3);
    if (v[0]) {
        v[1] /= v[0];
        v[2] /= v[0];
        v[3] /= v[0];
        q[0] = v[1] * v1[0] + v[2] * v1[1] + v[3] * v1[2];
        q[1] = v1[1] * v[3] - v[2] * v1[2];
        q[2] = v1[2] * v[1] - v[3] * v1[0];
        q[3] = v1[0] * v[2] - v[1] * v1[1];
    } else {
        q[0] = 0;
        q[1] = 1;
        q[2] = 0;
        q[3] = 0;
    }
}

static void qMultiVec(
        ESKF_3dofcute_fp_type *const vector,
        ESKF_3dofcute_fp_type *const vec,
        ESKF_3dofcute_fp_type *const q
) {
    ESKF_3dofcute_fp_type x, y, z, w;
    ESKF_3dofcute_fp_type x_, y_, z_, w_;
    ESKF_3dofcute_fp_type vecx, vecy, vecz;
    x = q[1];
    y = q[2];
    z = q[3];
    w = q[0];

    vecx = vec[0];
    vecy = vec[1];
    vecz = vec[2];

    x_ = w * vecx + y * vecz - z * vecy;
    y_ = w * vecy + z * vecx - x * vecz;
    z_ = w * vecz + x * vecy - y * vecx;
    w_ = -x * vecx - y * vecy - z * vecz;

    vector[0] = x_ * w + w_ * -x + y_ * -z - z_ * -y;
    vector[1] = y_ * w + w_ * -y + z_ * -x - x_ * -z;
    vector[2] = z_ * w + w_ * -z + x_ * -y - y_ * -x;

}

static void qMultiQ(
        ESKF_3dofcute_fp_type *const qq,
        ESKF_3dofcute_fp_type *const p,
        ESKF_3dofcute_fp_type *const q
) {
    qq[0] = p[0] * q[0] - p[1] * q[1] - p[2] * q[2] - p[3] * q[3];
    qq[1] = p[1] * q[0] + p[0] * q[1] - p[3] * q[2] + p[2] * q[3];
    qq[2] = p[2] * q[0] + p[3] * q[1] + p[0] * q[2] - p[1] * q[3];
    qq[3] = p[3] * q[0] - p[2] * q[1] + p[1] * q[2] + p[0] * q[3];
}

static void rotMat2qRichard(
        ESKF_3dofcute_fp_type *const q,
        ESKF_3dofcute_fp_type *const R
) {
    ESKF_3dofcute_fp_type vX[3];
    ESKF_3dofcute_fp_type vY[3];
    ESKF_3dofcute_fp_type qX[4];
    ESKF_3dofcute_fp_type qY[4];
    ESKF_3dofcute_fp_type y[3];

    vX[0] = R[0];
    vX[1] = R[3];
    vX[2] = R[6];

    vY[0] = R[1];
    vY[1] = R[4];
    vY[2] = R[7];

    qUtoV(qX, vX, 0);

    qMultiVec(y, vY, qX);

    qUtoV(qY, y, 1);

    qX[0] = -qX[0];
    qY[0] = -qY[0];

    qMultiQ(q, qX, qY);

}

static void getInitQuaternionfromAcc(
        ESKF_3dofcute_fp_type* const Q,
        ESKF_3dofcute_fp_type const acc_x,
        ESKF_3dofcute_fp_type const acc_y,
        ESKF_3dofcute_fp_type const acc_z
) {
    ESKF_3dofcute_fp_type buf3[4];
    ESKF_3dofcute_fp_type buf[9];
    int i;
    buf3[1] = acc_x;
    buf3[2] = acc_y;
    buf3[3] = acc_z;

    buf3[0] = norm(buf3 + 1, 3);

    for (i = 0; i < 9; ++i) {
        buf[i] = 0;
    }

    buf3[1] /= buf3[0];
    buf3[2] /= buf3[0];
    buf3[3] /= buf3[0];

    buf[6] = buf3[1];
    buf[7] = buf3[2];
    buf[8] = buf3[3];

    if ( buf[7] >init_angle_threshold || buf[7] < -1 * init_angle_threshold) {
//        LOGD("eskf 1111 %.2f %.2f %.2f",buf[6],buf[7],buf[8]);
        buf[0] = sqrt(1 - buf3[1] * buf3[1]);
        buf[3] = 0;
        buf[1] = -buf[6] * buf[7] / buf[0];
        buf[2] = -buf[6] * buf[8] / buf[0];
        buf[4] = buf[8] / buf[0];
        buf[5] = -buf[7] / buf[0];
        if (buf[4] * (buf[0] * buf[8] - buf[2] * buf[6]) + buf[5] * (buf[1] * buf[6] - buf[0] * buf[7]) <= 0) {
            buf[4] = -buf[4];
            buf[5] = -buf[5];
        }
        rotMat2qRichard(Q, buf);
        LOGD("eskf 1111 %.4f %.4f %.4f",buf[6],buf[7],buf[8]);
        LOGD("eskf 1111 %.4f %.4f %.4f %.4f",Q[0],Q[1],Q[2],Q[3]);
    }else{
        buf[1] = 0;
        buf[4] = sqrt(1 - buf3[2] * buf3[2]);
        buf[3] = -buf3[1] * buf3[2] / buf[4];
        buf[5] = -buf3[2] * buf3[3] / buf[4];
        buf[0] = buf3[3] / buf[4];
        buf[2] = -buf3[1] / buf[4];
        if (buf[0] * (buf[4] * buf3[3] - buf[5] * buf3[2]) + buf[2] * (buf[3] * buf3[2] - buf[4] * buf3[1]) <= 0) {
            buf[0] = -buf[0];
            buf[2] = -buf[2];
        }
        rotMat2qRichard(Q, buf);
    }


}

static const double Para_delta_theta0 = 0.00001;
static const double Para_delta_theta = 0.0001;
static const double para_Rk_acc = 0.1;
static const int para_memcount = ESKF_3dofcute_fp_count;
static const double para_Vgx = 0;
static const double para_Vgy = 0;
static const double para_Vgz = 1;
static const double para_t = 0.01;

//int dump(ESKF_3dofcute_fp_type* ptr, int m, int limit) {
//	int i;
//	for (i = 0; i < limit * m; ++i) {
//		if (i % m == 0) {
//			printf("\n");
//		}
//		printf("%f ", ptr[i]);
//	}
//	printf("\n");
//	return 0;
//}
void ESKF_3dofcute_status_new(ESKF_3dofcute_status **const ret) {

    ESKF_3dofcute_fp_type *ptr;
    ESKF_3dofcute_status *q;
    int i;
    q = (ESKF_3dofcute_status *) malloc(sizeof(ESKF_3dofcute_status));

    if (!q) {
        *ret = 0;
        return;
    }

    ptr = (ESKF_3dofcute_fp_type *) malloc(sizeof(ESKF_3dofcute_fp_type) * para_memcount);

    if (!ptr) {
        free(q);
        *ret = 0;
        return;
    }
    for (i = 0; i < para_memcount; ++i) {
        ptr[i] = 0;
    }
    q->raw_array = ptr;
    //ptr += offset;
    q->Q_temp = ptr += 4;
    q->Q_prev = ptr += 4;
    q->Q_curr = ptr += 4;
    q->Vg = ptr += 4;
    q->Pk_prev = ptr += 16;
    q->Pk_curr = ptr += 16;
    q->buf = ptr += 16;
    q->buf2 = ptr += 16;
    q->buf3 = ptr += 16;
    q->Q_delta_X = ptr += 16;
    q->F_delta_X = ptr += 16;
    q->S = ptr += 16;
    q->SS = ptr += 16;
    q->E3 = ptr += 16;
    q->J1 = ptr += 16;
    q->J2 = ptr += 16;
    q->J3 = ptr += 16;
    q->X_delta_x = ptr += 16;
    q->Hk = ptr += 16;
    q->P_k = ptr += 16;
    q->Pk2_ = ptr += 16;
    q->Kk = ptr += 16;
    q->G = ptr += 16;
    q->H_acc = ptr += 16;
    q->Rk = ptr += 16;
    q->h_acc = ptr += 16;
    q->R1 = ptr += 16;
    q->R2 = ptr += 16;
    q->R3 = ptr += 16;
    q->Qp = ptr += 16;
    q->delta_X_prev = ptr += 16;
    q->delta_X_curr = ptr += 16;
    q->delta_Xp = ptr += 16;
    q->delta_X_hat = ptr += 16;
    q->u = ptr += 16;


    q->Q_curr[0] = 1;
    eye(q->Pk_curr, 3, Para_delta_theta0);
    eye(q->Q_delta_X, 3, Para_delta_theta);
    eye(q->Rk, 3, para_Rk_acc);
    eye(q->E3, 3, 1);


    q->Vg[0] = para_Vgx;
    q->Vg[1] = para_Vgy;
    q->Vg[2] = para_Vgz;

    q->t = para_t;

    *ret = q;
}

void ESKF_3dofcute_status_delete(ESKF_3dofcute_status **const ptr) {
    free((*ptr)->raw_array);
    free(*ptr);
    *ptr = 0;
}

void ESKF_3dofcute_status_init(
        ESKF_3dofcute_fp_type *Q_result,
        ESKF_3dofcute_status *const ptr,
        ESKF_3dofcute_fp_type const acc_x,
        ESKF_3dofcute_fp_type const acc_y,
        ESKF_3dofcute_fp_type const acc_z
) {
    ESKF_3dofcute_fp_type *const Q = ptr->Q_curr;
    eye(ptr->Pk_curr, 3, Para_delta_theta0);
    getInitQuaternionfromAcc(ptr->Q_curr, acc_x, acc_y, acc_z);
    Q_result[0] = ptr->Q_curr[0];
    Q_result[1] = ptr->Q_curr[1];
    Q_result[2] = ptr->Q_curr[2];
    Q_result[3] = ptr->Q_curr[3];
}

void ESKF_3dofcute_step(
        ESKF_3dofcute_fp_type *Q_result,
        ESKF_3dofcute_status *const status,
        ESKF_3dofcute_fp_type const acc_x,
        ESKF_3dofcute_fp_type const acc_y,
        ESKF_3dofcute_fp_type const acc_z,
        ESKF_3dofcute_fp_type const gyro_x,
        ESKF_3dofcute_fp_type const gyro_y,
        ESKF_3dofcute_fp_type const gyro_z
) {
    ESKF_3dofcute_fp_type *const buf = status->buf;
    ESKF_3dofcute_fp_type *const buf2 = status->buf2;
    ESKF_3dofcute_fp_type *const Vg = status->Vg;
    ESKF_3dofcute_fp_type *const Qp = status->Qp;
    ESKF_3dofcute_fp_type *const X_delta_x = status->X_delta_x;
    ESKF_3dofcute_fp_type *const R1 = status->R1;
    ESKF_3dofcute_fp_type *const R2 = status->R2;
    ESKF_3dofcute_fp_type *const R3 = status->R3;
    ESKF_3dofcute_fp_type *const J1 = status->J1;
    ESKF_3dofcute_fp_type *const J2 = status->J2;
    ESKF_3dofcute_fp_type *const J3 = status->J3;
    ESKF_3dofcute_fp_type *swap_a;
    ESKF_3dofcute_fp_type *swap_b;
    ESKF_3dofcute_fp_type len;


    buf[0] = acc_x;
    buf[1] = acc_y;
    buf[2] = acc_z;
    status->norm_a = norm(buf, 3);

    buf[0] = gyro_x;
    buf[1] = gyro_y;
    buf[2] = gyro_z;
    status->norm_g = norm(buf, 3);


    if (status->norm_g == 0.0) {

        status->delta_X_prev[0] = 0;
        status->delta_X_prev[1] = 0;
        status->delta_X_prev[2] = 0;


        swap_a = status->Q_curr;
        swap_b = status->Q_prev;
        status->Q_curr = swap_b;
        status->Q_prev = swap_a;

        swap_a = status->Pk_curr;
        swap_b = status->Pk_prev;
        status->Pk_curr = swap_b;
        status->Pk_prev = swap_a;

        swap_a = status->delta_X_curr;
        swap_b = status->delta_X_prev;
        status->delta_X_curr = swap_b;
        status->delta_X_prev = swap_a;
    } else {

        status->u[0] = gyro_x * status->t;
        status->u[1] = gyro_y * status->t;
        status->u[2] = gyro_z * status->t;

        buf2[0] = status->u[0] / 2;
        buf2[1] = status->u[1] / 2;
        buf2[2] = status->u[2] / 2;

        buf2[3 + 0] = -buf2[0];
        buf2[3 + 1] = -buf2[1];
        buf2[3 + 2] = -buf2[2];

        buf[0] = 1;
        buf[1] = buf2[3 + 0];
        buf[2] = buf2[3 + 1];
        buf[3] = buf2[3 + 2];

        buf[4] = buf2[0];
        buf[5] = 1;
        buf[6] = buf2[2];
        buf[7] = buf2[3 + 1];

        buf[8] = buf2[1];
        buf[9] = buf2[3 + 2];
        buf[10] = 1;
        buf[11] = buf2[0];

        buf[12] = buf2[2];
        buf[13] = buf2[1];
        buf[14] = buf2[3 + 0];
        buf[15] = 1;

        mat_mul(Qp, buf, status->Q_curr, 4, 4, 1);
        len = norm(Qp, 4);
        Qp[0] /= len;
        Qp[1] /= len;
        Qp[2] /= len;
        Qp[3] /= len;

        len = norm(status->u, 3);

        if (len > 0) {
            status->u[0] /= len;
            status->u[1] /= len;
            status->u[2] /= len;
            Skew_symmetric(status->S, status->u);
            mat_mul(status->SS, status->S, status->S, 3, 3, 3);
            mat_add(buf, status->S, status->SS, 3, 3, -sin(len), 1 - cos(len));
            mat_add(status->F_delta_X, buf, status->E3, 3, 3, 1, 1);
        } else {
            eye(status->F_delta_X, 3, 1);
        }

        mat_mul(status->delta_Xp, status->F_delta_X, status->delta_X_curr, 3, 3, 1);

        mat_t(buf, status->F_delta_X, 3, 3);
        mat_mul(buf2, status->Pk_curr, buf, 3, 3, 3);
        mat_mul(buf, status->F_delta_X, buf2, 3, 3, 3);
        mat_add(status->P_k, buf, status->Q_delta_X, 3, 3, 1, 1);


        if (1) {//(status->norm_a < 11.8 && status->norm_a>7.8 && status->norm_g < 1) {

            R1[0] = 2 * Qp[0] * Qp[0] + 2 * Qp[1] * Qp[1] - 1;
            R1[1] = 2 * Qp[1] * Qp[2] - 2 * Qp[0] * Qp[3];
            R1[2] = 2 * Qp[0] * Qp[2] + 2 * Qp[1] * Qp[3];

            R2[0] = 2 * Qp[1] * Qp[2] + 2 * Qp[0] * Qp[3];
            R2[1] = 2 * Qp[0] * Qp[0] + 2 * Qp[2] * Qp[2] - 1;
            R2[2] = 2 * Qp[2] * Qp[3] - 2 * Qp[0] * Qp[1];

            R3[0] = 2 * Qp[1] * Qp[3] - 2 * Qp[0] * Qp[2];
            R3[1] = 2 * Qp[2] * Qp[3] + 2 * Qp[0] * Qp[1];
            R3[2] = 2 * Qp[0] * Qp[0] + 2 * Qp[3] * Qp[3] - 1;

            mat_add(buf, R1, R2, 3, 1, Vg[0], Vg[1]);
            mat_add(status->h_acc, buf, R3, 3, 1, 1, Vg[2]);


            buf[0] = Qp[0] / 2;
            buf[1] = Qp[1] / 2;
            buf[2] = Qp[2] / 2;
            buf[3] = Qp[3] / 2;
            buf[4] = -buf[0];
            buf[5] = -buf[1];
            buf[6] = -buf[2];
            buf[7] = -buf[3];

            X_delta_x[0] = buf[4 + 1];
            X_delta_x[1] = buf[4 + 2];
            X_delta_x[2] = buf[4 + 3];
            X_delta_x[3] = buf[0];
            X_delta_x[4] = buf[4 + 3];
            X_delta_x[5] = buf[2];
            X_delta_x[6] = buf[3];
            X_delta_x[7] = buf[0];
            X_delta_x[8] = buf[4 + 1];
            X_delta_x[9] = buf[4 + 2];
            X_delta_x[10] = buf[1];
            X_delta_x[11] = buf[0];

            buf[0] *= 4;
            buf[1] *= 4;
            buf[2] *= 4;
            buf[3] *= 4;
            buf[4] = -buf[0];
            buf[5] = -buf[1];
            buf[6] = -buf[2];
            buf[7] = -buf[3];

            J1[0] = buf[0];
            J1[1] = buf[1];
            J1[2] = buf[4 + 2];
            J1[3] = buf[4 + 3];
            J1[4] = buf[4 + 3];
            J1[5] = buf[2];
            J1[6] = buf[1];
            J1[7] = buf[4 + 0];
            J1[8] = buf[2];
            J1[9] = buf[3];
            J1[10] = buf[0];
            J1[11] = buf[1];

            J2[0] = buf[3];
            J2[1] = buf[2];
            J2[2] = buf[1];
            J2[3] = buf[0];
            J2[4] = buf[0];
            J2[5] = buf[4 + 1];
            J2[6] = buf[2];
            J2[7] = buf[4 + 3];
            J2[8] = buf[4 + 1];
            J2[9] = buf[4 + 0];
            J2[10] = buf[3];
            J2[11] = buf[2];

            J3[0] = buf[4 + 2];
            J3[1] = buf[3];
            J3[2] = buf[4 + 0];
            J3[3] = buf[1];
            J3[4] = buf[1];
            J3[5] = buf[0];
            J3[6] = buf[3];
            J3[7] = buf[2];
            J3[8] = buf[0];
            J3[9] = buf[4 + 1];
            J3[10] = buf[4 + 2];
            J3[11] = buf[3];

            mat_add(buf, J1, J2, 3, 4, Vg[0], Vg[1]);
            mat_add(status->H_acc, buf, J3, 3, 4, 1, Vg[2]);

            mat_mul(status->Hk, status->H_acc, X_delta_x, 3, 4, 3);

            mat_t(status->buf3, status->Hk, 3, 3);
            mat_mul(buf2, status->P_k, status->buf3, 3, 3, 3);
            mat_mul(buf, status->Hk, buf2, 3, 3, 3);

            eye(status->Rk, 3, (para_Rk_acc + 300 * status->norm_g) * 1000);

            //printf("\t%f\t%f\t%f\t%f\t%f\t%f\t%f", status->Rk[0], status->Rk[4], status->Rk[8], status->norm_g, gyro_x, gyro_y, gyro_z);
            //printf("\n");


            mat_add(buf2, buf, status->Rk, 3, 3, 1, 1);
            mat_inv3(buf, buf2);
            mat_mul(buf2, status->buf3, buf, 3, 3, 3);
            mat_mul(status->Kk, status->P_k, buf2, 3, 3, 3);

            buf2[0] = acc_x / status->norm_a;
            buf2[1] = acc_y / status->norm_a;
            buf2[2] = acc_z / status->norm_a;
            mat_add(buf, buf2, status->h_acc, 3, 1, 1, -1);
            mat_mul(status->delta_X_hat, status->Kk, buf, 3, 3, 1);

            buf[0] = 1;
            buf[1] = status->delta_X_hat[0] / 2;
            buf[2] = status->delta_X_hat[1] / 2;
            buf[3] = status->delta_X_hat[2] / 2;

            quaternProd(status->Q_prev, Qp, buf);

            len = norm(status->Q_prev, 4);
            status->Q_prev[0] /= len;
            status->Q_prev[1] /= len;
            status->Q_prev[2] /= len;
            status->Q_prev[3] /= len;

            mat_mul(buf, status->Kk, status->Hk, 3, 3, 3);
            mat_add(buf2, status->E3, buf, 3, 3, 1, -1);
            mat_mul(status->Pk2_, buf2, status->P_k, 3, 3, 3);

            mat_add(status->delta_X_prev, status->delta_Xp, status->delta_X_hat, 3, 1, 1, 1);

            buf[0] = -status->delta_X_prev[0] / 2;
            buf[1] = -status->delta_X_prev[1] / 2;
            buf[2] = -status->delta_X_prev[2] / 2;

            Skew_symmetric(status->S, buf);

            mat_add(status->G, status->E3, status->S, 3, 3, 1, -1);

            mat_t(buf2, status->G, 3, 3);
            mat_mul(buf, status->Pk2_, buf2, 3, 3, 3);
            mat_mul(status->Pk_prev, status->G, buf, 3, 3, 3);

        } else {
            status->Q_prev[0] = Qp[0];
            status->Q_prev[1] = Qp[1];
            status->Q_prev[2] = Qp[2];
            status->Q_prev[3] = Qp[3];

            status->Pk_prev[0] = status->P_k[0];
            status->Pk_prev[1] = status->P_k[1];
            status->Pk_prev[2] = status->P_k[2];
            status->Pk_prev[3] = status->P_k[3];
            status->Pk_prev[4] = status->P_k[4];
            status->Pk_prev[5] = status->P_k[5];
            status->Pk_prev[6] = status->P_k[6];
            status->Pk_prev[7] = status->P_k[7];
            status->Pk_prev[8] = status->P_k[8];

            status->delta_X_prev[0] = status->delta_Xp[0];
            status->delta_X_prev[1] = status->delta_Xp[1];
            status->delta_X_prev[2] = status->delta_Xp[2];
        }
    }
    status->delta_X_prev[0] = 0;
    status->delta_X_prev[1] = 0;
    status->delta_X_prev[2] = 0;


    swap_a = status->Q_curr;
    swap_b = status->Q_prev;
    status->Q_curr = swap_b;
    status->Q_prev = swap_a;

    swap_a = status->Pk_curr;
    swap_b = status->Pk_prev;
    status->Pk_curr = swap_b;
    status->Pk_prev = swap_a;

    swap_a = status->delta_X_curr;
    swap_b = status->delta_X_prev;
    status->delta_X_curr = swap_b;
    status->delta_X_prev = swap_a;

    Q_result[0] = status->Q_curr[0];
    Q_result[1] = status->Q_curr[1];
    Q_result[2] = status->Q_curr[2];
    Q_result[3] = status->Q_curr[3];
}

void vector2angle(ESKF_3dofcute_fp_type *const result,
                  ESKF_3dofcute_fp_type V1,
                  ESKF_3dofcute_fp_type V2,
                  ESKF_3dofcute_fp_type V3) {


    ESKF_3dofcute_fp_type normV = sqrt(V1 * V1 + V2 * V2 + V3 * V3);

    V3 = V3 / normV;

    ESKF_3dofcute_fp_type theta;
    ESKF_3dofcute_fp_type phi;


    if (V3 == 1) {
        theta = 0;
        phi = 0;

    } else if (V3 == -1) {
        theta = 0;
        phi = 180;

    } else {

        phi = acos(V3) / M_PI * 180;

        ESKF_3dofcute_fp_type normVxy = sqrt(V1 * V1 + V2 * V2);

        V1 = V1 / normVxy;
        V2 = V2 / normVxy;

        theta = acos(V1) / M_PI * 180;

        if (V2 < 0) {

            theta = 360 - theta;
        }


    }

    result[0] = theta;

    result[1] = phi;

}


ESKF_3dofcute_fp_type MeanArray(ESKF_3dofcute_fp_type array[], int n) {
    int i;
    ESKF_3dofcute_fp_type sum = 0;

    for (i = 0; i < n; ++i) {
        sum += array[i];
    }
    ESKF_3dofcute_fp_type mean = sum / n;

    return mean;
}


void computeBgOnlineMean(
        ESKF_3dofcute_fp_type gyro_raw_x, ESKF_3dofcute_fp_type gyro_raw_y,
        ESKF_3dofcute_fp_type gyro_raw_z) {

//	printf("\t%f\t%f\t%f\t%f", a[0], a[1], b[0], b[1]);

    ESKF_3dofcute_fp_type wx1 = gyro_raw_x - wx2 * a[1];
    ESKF_3dofcute_fp_type gyro_x_Filt = wx1 * b[0] + wx2 * b[1];
    wx2 = wx1;

    ESKF_3dofcute_fp_type wy1 = gyro_raw_y - wy2 * a[1];
    ESKF_3dofcute_fp_type gyro_y_Filt = wy1 * b[0] + wy2 * b[1];
    wy2 = wy1;

    ESKF_3dofcute_fp_type wz1 = gyro_raw_z - wz2 * a[1];
    ESKF_3dofcute_fp_type gyro_z_Filt = wz1 * b[0] + wz2 * b[1];
    wz2 = wz1;

    ESKF_3dofcute_fp_type temp_sqt_gyro =
            gyro_x_Filt * gyro_x_Filt + gyro_y_Filt * gyro_y_Filt + gyro_z_Filt * gyro_z_Filt;


    temp_n = temp_n % (n_win * 2 + 1);

    if (temp_n == n_win * 2) {
        flag = 1;
    }


    ESKF_3dofcute_fp_type table_gyro_raw_temp_n0 = table_gyro_raw0[temp_n];
    ESKF_3dofcute_fp_type table_gyro_raw_temp_n1 = table_gyro_raw1[temp_n];
    ESKF_3dofcute_fp_type table_gyro_raw_temp_n2 = table_gyro_raw2[temp_n];
    ESKF_3dofcute_fp_type table_gyro_raw_temp_n3 = table_gyro_raw3[temp_n];
    ESKF_3dofcute_fp_type table_gyro_raw_temp_n4 = table_gyro_raw4[temp_n];

    table_gyro_raw0[temp_n] = gyro_x_Filt;
    table_gyro_raw1[temp_n] = gyro_y_Filt;
    table_gyro_raw2[temp_n] = gyro_z_Filt;
    table_gyro_raw3[temp_n] = temp_sqt_gyro * temp_sqt_gyro;
    table_gyro_raw4[temp_n] = temp_sqt_gyro;

    if (flag == 1) {
        if (flagmean == 0) {
            table_mean0 = MeanArray(table_gyro_raw0, n_win * 2 + 1);
            table_mean1 = MeanArray(table_gyro_raw1, n_win * 2 + 1);
            table_mean2 = MeanArray(table_gyro_raw2, n_win * 2 + 1);
            table_mean3 = MeanArray(table_gyro_raw3, n_win * 2 + 1);
            table_mean4 = MeanArray(table_gyro_raw4, n_win * 2 + 1);

            flagmean = 1;

        } else {
            table_mean0 = table_mean0 - table_gyro_raw_temp_n0 / (n_win * 2 + 1) +
                          table_gyro_raw0[temp_n] / (n_win * 2 + 1);
            table_mean1 = table_mean1 - table_gyro_raw_temp_n1 / (n_win * 2 + 1) +
                          table_gyro_raw1[temp_n] / (n_win * 2 + 1);
            table_mean2 = table_mean2 - table_gyro_raw_temp_n2 / (n_win * 2 + 1) +
                          table_gyro_raw2[temp_n] / (n_win * 2 + 1);
            table_mean3 = table_mean3 - table_gyro_raw_temp_n3 / (n_win * 2 + 1) +
                          table_gyro_raw3[temp_n] / (n_win * 2 + 1);
            table_mean4 = table_mean4 - table_gyro_raw_temp_n4 / (n_win * 2 + 1) +
                          table_gyro_raw4[temp_n] / (n_win * 2 + 1);

        }

        ESKF_3dofcute_fp_type var_gyro_mag =
                (table_mean3 - table_mean4 * table_mean4) * (n_win * 2 + 1) / (2 * n_win);

        if (var_gyro_mag < 40) {
            n += 1;

            if (n > 2001) {
                n = 2001;
            }

            Bgx = Bgx * (n - 1) / n + table_mean0 / n;
            Bgy = Bgy * (n - 1) / n + table_mean1 / n;
            Bgz = Bgz * (n - 1) / n + table_mean2 / n;

            /*
            for (int s = 0; s++; s < 401)
            {

                printf("\t%f\t%f\t%f\t%f\t%f", table_gyro_raw0[s], table_gyro_raw1[s], table_gyro_raw2[s], table_gyro_raw3[s], table_gyro_raw4[s]);
                printf("\n");

            }*/

            int aaa = 10;


        }
    }
}

void getIntrinsics(double *accScale, double *accOffset, double *gyroScale, double *gyroOffset) {
    accScale[0] = Kax;
    accScale[1] = Kay;
    accScale[2] = Kaz;

    accOffset[0] = Bax;
    accOffset[1] = Bay;
    accOffset[2] = Baz;

    gyroScale[0] = Kgx;
    gyroScale[1] = Kgy;
    gyroScale[2] = Kgz;

    gyroOffset[0] = Bgx;
    gyroOffset[1] = Bgy;
    gyroOffset[2] = Bgz;

}

void initIntrinsics(
        ESKF_3dofcute_fp_type const Bax_param, ESKF_3dofcute_fp_type const Bay_param,
        ESKF_3dofcute_fp_type const Baz_param,
        ESKF_3dofcute_fp_type const Kax_param, ESKF_3dofcute_fp_type const Kay_param,
        ESKF_3dofcute_fp_type const Kaz_param,
        ESKF_3dofcute_fp_type const Bgx_param, ESKF_3dofcute_fp_type const Bgy_param,
        ESKF_3dofcute_fp_type const Bgz_param,
        ESKF_3dofcute_fp_type const Kgx_param, ESKF_3dofcute_fp_type const Kgy_param,
        ESKF_3dofcute_fp_type const Kgz_param) {
    Bax = Bax_param;
    Bay = Bay_param;
    Baz = Baz_param;

    Kax = Kax_param;
    Kay = Kay_param;
    Kaz = Kaz_param;

    Bgx = Bgx_param;
    Bgy = Bgy_param;
    Bgz = Bgz_param;

    Kgx = Kgx_param;
    Kgy = Kgy_param;
    Kgz = Kgz_param;

}

void calculateAccAndGyro(
        ESKF_3dofcute_fp_type *ACC_result, ESKF_3dofcute_fp_type *GYRO_result,
        ESKF_3dofcute_fp_type const acc_x,
        ESKF_3dofcute_fp_type const acc_y,
        ESKF_3dofcute_fp_type const acc_z,
        ESKF_3dofcute_fp_type const gyro_x,
        ESKF_3dofcute_fp_type const gyro_y,
        ESKF_3dofcute_fp_type const gyro_z) {
    ESKF_3dofcute_fp_type acc_x_now = (acc_x - Bax) * Kax;
    ESKF_3dofcute_fp_type acc_y_now = (acc_y - Bay) * Kay;
    ESKF_3dofcute_fp_type acc_z_now = (acc_z - Baz) * Kaz;

    ESKF_3dofcute_fp_type gyro_x_now = (gyro_x - Bgx) * Kgx;
    ESKF_3dofcute_fp_type gyro_y_now = (gyro_y - Bgy) * Kgy;
    ESKF_3dofcute_fp_type gyro_z_now = (gyro_z - Bgz) * Kgz;

    temp_n += 1;
    computeBgOnlineMean(gyro_x, gyro_y, gyro_z);

    ACC_result[0] = acc_x_now;
    ACC_result[1] = acc_y_now;
    ACC_result[2] = acc_z_now;

    GYRO_result[0] = gyro_x_now;
    GYRO_result[1] = gyro_y_now;
    GYRO_result[2] = gyro_z_now;
}

void get_Screen_Attitude(
        ESKF_3dofcute_fp_type*  Qcsreen,
        ESKF_3dofcute_fp_type*  Pcsreen,
        ESKF_3dofcute_fp_type*   q)
{

    ESKF_3dofcute_fp_type  R[9];

    quatern2rotMat(R, q);

//    if (R[4] > 0.984807753012208){
//    if (R[4] >  0.939692620785908){//20
//    LOGD("error error -------%f",R[4]);
    if (R[4] > 0.965925826289068) {//15
//    if (R[4] >  0.984807753012208){//10
//    if (R[4] >  0.996194698091746){//5

        Pcsreen[0] = 0;
        Pcsreen[1] = 1;
        Pcsreen[2] = 0;

        Qcsreen[0] = 1;
        Qcsreen[1] = 0;
        Qcsreen[2] = 0;
        Qcsreen[3] = 0;
    } else {

        Pcsreen[0] = R[1];
        Pcsreen[1] = R[4];
        Pcsreen[2] = R[7];

        if (R[7] > init_angle_threshold || R[7] < -1 * init_angle_threshold) {

            Qcsreen[0] = q[0];
            Qcsreen[1] = q[1];
            Qcsreen[2] = q[2];
            Qcsreen[3] = q[3];

        } else {

            ESKF_3dofcute_fp_type Vx[3];
            ESKF_3dofcute_fp_type Vy[3];
            ESKF_3dofcute_fp_type Vz[3];

            Vy[0] = R[1];
            Vy[1] = R[4];
            Vy[2] = R[7];

            Vz[0] = 0;
            Vz[1] = 0;
            Vz[2] = 1;

            cross3(Vx, Vy, Vz);

            ESKF_3dofcute_fp_type normVx = norm(Vx, 3);

            Vx[0] /= normVx;
            Vx[1] /= normVx;
            Vx[2] /= normVx;

            cross3(Vz, Vx, Vy);

            R[0] = Vx[0];
            R[1] = Vy[0];
            R[2] = Vz[0];
            R[3] = Vx[1];
            R[4] = Vy[1];
            R[5] = Vz[1];
            R[6] = Vx[2];
            R[7] = Vy[2];
            R[8] = Vz[2];

            rotMat2qRichard(Qcsreen, R);

        }
    }

}

void changePscreen2Unity(
        ESKF_3dofcute_fp_type* PscreenUnity,
        ESKF_3dofcute_fp_type* Pscreen
)
{
    PscreenUnity[0] = Pscreen[0];
    PscreenUnity[1] = Pscreen[2];
    PscreenUnity[2] = Pscreen[1];

}

void changeQscreen2Unity(
        ESKF_3dofcute_fp_type* QscreenUnity,
        ESKF_3dofcute_fp_type* Qscreen
)
{
    QscreenUnity[0] = Qscreen[0];
    QscreenUnity[1] = -Qscreen[1];
    QscreenUnity[2] = -Qscreen[3];
    QscreenUnity[3] = -Qscreen[2];

}



#ifdef ESKF_3dofcute_DEMO

/*
int main() {






	*/
/*
	ESKF_3dofcute_fp_type gyro_x_Filt = 0;
	ESKF_3dofcute_fp_type gyro_y_Filt = 0;
	ESKF_3dofcute_fp_type gyro_z_Filt = 0;

	ESKF_3dofcute_fp_type temp_sqt_gyro = 0;
	ESKF_3dofcute_fp_type var_gyro_mag = 0;
	*//*




	FILE* fin;
	char str[256];
	int T;
	ESKF_3dofcute_fp_type input[8];
	int i;
	int doReset = 0;
	int inited = 0;
	ESKF_3dofcute_fp_type R_result[16];
	ESKF_3dofcute_fp_type Q_result[4];
	ESKF_3dofcute_status* status;
	ESKF_3dofcute_fp_type IMU_data[8];
	ESKF_3dofcute_fp_type x, y;
	ESKF_3dofcute_fp_type acc_x_now = 0.0, acc_y_now = 0.0 , acc_z_now = 0.0;
	ESKF_3dofcute_fp_type gyro_x_now = 0.0, gyro_y_now = 0.0, gyro_z_now = 0.0;
	ESKF_3dofcute_fp_type t_now = 0.0;
	ESKF_3dofcute_fp_type t_old = 0.0;

	ESKF_3dofcute_status_new(&status);
	//errno_t t = fopen_s(&fin, "2-shufang.txt", "r");
	errno_t t = fopen_s(&fin, "IMU.1678439027231_.txt", "r");
	//fprintf("error %d", strerror(t));
	//fgets(str, 256, fin);

	for (T = 0; ; ++T) {

		for (i = 0; i < 256; ++i) {
			str[i] = 0;
		}
		fgets(str, 256, fin);
		i = sscanf(str, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", input + 0, input + 1, input + 2, input + 3, input + 4, input + 5, input + 6, input + 7);
		if (i != 8) {
			break;
		}
		for (int i = 0; i < 8; ++i) {
			IMU_data[i] = input[i];
		}
		if (doReset) {
			x = 0;
			y = 0;
			inited = 0;
		}
		if (status)
		{

			acc_x_now = (IMU_data[2] - Bax) * Kax;
			acc_y_now = (IMU_data[3] - Bay) * Kay;
			acc_z_now = (IMU_data[4] - Baz) * Kaz;

			gyro_x_now = (IMU_data[5] - Bgx) * Kgx;
			gyro_y_now = (IMU_data[6] - Bgy) * Kgy;
			gyro_z_now = (IMU_data[7] - Bgz) * Kgz;




			t_old = t_now;

			t_now = IMU_data[1]*10e-10;

			temp_n = temp_n + 1;

			computeBgOnlineMean(
					IMU_data[5], IMU_data[6],
					IMU_data[7]);



			printf("Bg=\t%f\t%f\t%f", Bgx,Bgy,Bgz );
			printf("\n");

			//printf("\t%f\t%f\t%f\t%f", gyro_raw_now[0], IMU_data[5], IMU_data[6], IMU_data[7]);
			//printf("\n");

			int aaaa = 10;



			if (inited) {


				status->t = t_now - t_old;

				//printf("\t%f", status->t);
				//printf("\n");

				ESKF_3dofcute_step(Q_result, status, acc_x_now, acc_y_now , acc_z_now, gyro_x_now, gyro_y_now , gyro_z_now);

			}
			else {
				ESKF_3dofcute_status_init(Q_result, status, acc_x_now, acc_y_now , acc_z_now);


				//printf("\t%f\t%f\t%f\t%f", IMU_data[1], IMU_data[2], IMU_data[3], IMU_data[4]);
				//printf("\n");

				//printf("\t%f\t%f\t%f", acc_x_now, acc_y_now, acc_z_now);
				//printf("\n");

				//printf("\t%f\t%f\t%f\t%f", Q_result[0], Q_result[1], Q_result[2], Q_result[3]);
				//printf("\n");
				//printf("\n");


				inited = 1;
			}
		}



		printf("%d", T + 1);
		printf("\t%f\t%f\t%f\t%f", Q_result[0], Q_result[1], Q_result[2], Q_result[3]);
		printf("\n");

		doReset = 0;
	}

	//printf("\t%f\t%f\t%f", Bgx, Bgy, Bgz);
	//printf("\n");

	ESKF_3dofcute_status_delete(&status);

	fclose(fin);
	system("pause");
	return 0;
}
*/

#endif
