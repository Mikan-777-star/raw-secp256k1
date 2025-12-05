
#include "secp256k1_utils.h"


int is_zero(const Point* point){
    return is_zero_value(&point->z);
}

void double_Jacobian(Point* result, const Point* A){
    if (is_zero_value(&A->y)) {
        // Y=0 の場合、接線は垂直になるので結果は無限遠点
        zero_UInt256(&result->x);
        zero_UInt256(&result->y);
        zero_UInt256(&result->z);
        return;
    }

    UInt256 X2, Y2, Y4, S, M, tmp;
    Point res;

    // 1. Calculate X^2, Y^2, Y^4
    mod_mul(&X2, &A->x, &A->x);
    mod_mul(&Y2, &A->y, &A->y);
    mod_mul(&Y4, &Y2, &Y2);

    // 2. Calculate S = 4XY^2
    //    = 2 * (2 * X * Y^2)
    mod_mul(&tmp, &A->x, &Y2);
    mod_add(&tmp, &tmp, &tmp); // 2XY^2
    mod_add(&S, &tmp, &tmp);   // 4XY^2

    // 3. Calculate M = 3X^2
    mod_add(&tmp, &X2, &X2);   // 2X^2
    mod_add(&M, &tmp, &X2);    // 3X^2

    // 4. Calculate X3 = M^2 - 2S
    mod_mul(&res.x, &M, &M);   // M^2
    mod_sub(&res.x, &res.x, &S);
    mod_sub(&res.x, &res.x, &S);

    // 5. Calculate Y3 = M(S - X3) - 8Y^4
    mod_sub(&tmp, &S, &res.x);
    mod_mul(&tmp, &tmp, &M);   // M(S - X3)
    
    // 8Y^4 を作る (3回倍加)
    UInt256 Y4_8;
    mod_add(&Y4_8, &Y4, &Y4); // 2Y^4
    mod_add(&Y4_8, &Y4_8, &Y4_8); // 4Y^4
    mod_add(&Y4_8, &Y4_8, &Y4_8); // 8Y^4
    
    mod_sub(&res.y, &tmp, &Y4_8);

    // 6. Calculate Z3 = 2YZ
    mod_mul(&tmp, &A->y, &A->z);
    mod_add(&res.z, &tmp, &tmp); // 2倍算

    *result = res;
}

void set_inf(Point* ptr){
    zero_UInt256(&ptr->z);
}


void add_Jacobian(Point* result, const Point* A,const Point* B){
    Point res;
    UInt256 U1, U2, S1, S2, H, r, aZ2, bZ2, H2, H3, tmp, U1H2, S1H3;
    if (is_zero(A)) {
        *result = *B;
        return;
    }
    if (is_zero(B)) {
        *result = *A;
        return;
    }
    //Initialization U1
    mod_mul(&bZ2, &B->z, &B->z);
    mod_mul(&U1, &A->x, &bZ2);

    //Initialization U2
    mod_mul(&aZ2, &A->z, &A->z);
    mod_mul(&U2, &B->x, &aZ2);

    //Initialization S1
    mod_mul(&tmp, &bZ2, &B->z);
    mod_mul(&S1, &A->y, &tmp);

    //Initialization S2
    mod_mul(&tmp, &aZ2, &A->z);
    mod_mul(&S2, &B->y, &tmp);

    //Initialization H
    mod_sub(&H, &U2, &U1);
    
    //Initialization r
    mod_sub(&r, &S2, &S1);

    if (is_zero_value(&H)) {
        if (is_zero_value(&r)) {
            // H=0 かつ r=0 ということは、XもYも同じ。つまり P == Q
            // 2倍算関数を呼ぶ
            double_Jacobian(result, A);
            return;
        } else {
            // H=0 かつ r!=0 ということは、Xは同じでYが違う。つまり P == -Q
            // 結果は無限遠点
            set_inf(result);
            return;
        }
    }
    //Initialization H2
    mod_mul(&H2, &H, &H);

    //Initialization H3;
    mod_mul(&H3, &H2, &H);

    //Initialization U1H2;
    mod_mul(&U1H2, &U1, &H2);

    //Initialization S1H3
    mod_mul(&S1H3, &S1, &H3);

    //X3
    mod_mul(&tmp, &r, &r);
    mod_sub(&tmp, &tmp, &H3);
    mod_sub(&tmp, &tmp, &U1H2);
    mod_sub(&tmp, &tmp, &U1H2);

    res.x = tmp;

    //Y3
    mod_sub(&tmp, &U1H2, &res.x);
    mod_mul(&tmp, &tmp, &r);
    mod_sub(&tmp, &tmp, &S1H3);

    res.y = tmp;

    //Z3
    mod_mul(&tmp, &A->z, &B->z);
    mod_mul(&tmp, &tmp, &H);

    res.z = tmp;

    *result = res;
}


void mul_scalar(Point* result, const Point* P, const UInt256* k){
    Point R;
    set_inf(&R);

    for(int i = 255; i >= 0; i--){
        double_Jacobian(&R, &R);
        int bit = get_bit_UInt256(k, i);
        int mask = -bit;
        Point temp;
        add_Jacobian(&temp, &R, P);
        for(int j = 0; j < 4; j++)R.x.limbs[j] = (mask & temp.x.limbs[j]) | (~mask & R.x.limbs[j]);
        for(int j = 0; j < 4; j++)R.y.limbs[j] = (mask & temp.y.limbs[j]) | (~mask & R.y.limbs[j]);
        for(int j = 0; j < 4; j++)R.z.limbs[j] = (mask & temp.z.limbs[j]) | (~mask & R.z.limbs[j]);
    }
    *result = R;
}

void jacobian_to_affine(Point* affine_res, const Point* jacobian_pt){
    if(is_zero_value(&jacobian_pt->z)){
        zero_UInt256(&affine_res->x);
        zero_UInt256(&affine_res->y);
        return;
    }
    UInt256 z_inv, z_inv_2, z_inv_3, tmp;
    //Z_inv
    mod_inv(&z_inv, &jacobian_pt->z);

    //Z_inv_2
    mod_mul(&z_inv_2, &z_inv, &z_inv);
    
    //z_inv_3
    mod_mul(&z_inv_3, &z_inv_2, &z_inv);

    //x = X * Z^-1^2
    mod_mul(&affine_res->x, &jacobian_pt->x,  &z_inv_2);

    //y = Y * (Z^-1)^3
    mod_mul(&affine_res->y, &jacobian_pt->y, &z_inv_3);

    from_mont(&affine_res->x, &affine_res->x);
    from_mont(&affine_res->y, &affine_res->y);

    zero_UInt256(&affine_res->z);
    affine_res->z.limbs[0] = 0x1;
}