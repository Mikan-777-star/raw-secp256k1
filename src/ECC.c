
#include "secp256k1_utils.h"
#include <stdio.h>

// 簡易初期化関数
void init_point_G(Point* p) {
    p->x = G_X;
    p->y = G_Y;
    zero_UInt256(&p->z);
    p->z.limbs[0] = 1; // Z=1 (Affine -> Jacobian)
}

/**
 * 署名生成
 * r, s : 結果の格納先
 * z    : メッセージハッシュ (32byte数値)
 * d    : 秘密鍵
 * k    : エフェメラル鍵 (ランダムな値。定数ではない！)
 */
void ecdsa_sign(Signature* sig, const UInt256* z, const UInt256* d, const UInt256* k) {
    Point R_jac, G_jac;
    init_point_G(&G_jac);
    to_mont(&G_jac.x, &G_jac.x);
    to_mont(&G_jac.y, &G_jac.y);
    to_mont(&G_jac.z, &G_jac.z);

    mul_scalar(&R_jac, &G_jac, k);

    Point R_aff;
    jacobian_to_affine(&R_aff, &R_jac);
    sig->r = R_aff.x;
    sub_UInt256(&sig->r, &SECP256K1_N);
    sig->r = (cmp_UInt256(&R_aff.x, &SECP256K1_N) >= 0) ? sig->r : R_aff.x;

    UInt256 rd, z_rd, k_inv;

    //r * d
    scalar_mul(&rd, &sig->r, d);

    //z + rd
    scalar_add(&z_rd, z, &rd);

    scalar_inv(&k_inv, k);

    scalar_mul(&sig->s, &k_inv, &z_rd);
}
int ecdsa_verify(const Signature* sig, const UInt256* z, const Point* Q_aff){
    UInt256 w;
    scalar_inv(&w, &sig->s);

    UInt256 u1;
    scalar_mul(&u1, z, &w);

    UInt256 u2;
    scalar_mul(&u2, &sig->r, &w);

    Point G_jac, Q_jac, P1, P2, P_result;
    init_point_G(&G_jac);
    to_mont(&G_jac.x, &G_jac.x);
    to_mont(&G_jac.y, &G_jac.y);
    to_mont(&G_jac.z, &G_jac.z);

    // Qの準備 (Affine -> Jacobian -> モンゴメリ化)
    // Q_aff は通常の座標系で渡される想定
    Q_jac.x = Q_aff->x;
    Q_jac.y = Q_aff->y;
    zero_UInt256(&Q_jac.z);
    Q_jac.z.limbs[0] = 1;  //... Z=1
    to_mont(&Q_jac.x, &Q_jac.x);
    to_mont(&Q_jac.y, &Q_jac.y);
    to_mont(&Q_jac.z, &Q_jac.z);

    mul_scalar(&P1, &G_jac, &u1);

    mul_scalar(&P2, &Q_jac, &u2);

    add_Jacobian(&P_result, &P1, &P2);
    Point P_aff;

    jacobian_to_affine(&P_aff, &P_result);

    if(is_zero_value(&P_aff.z)){
        return 0;
    }

    UInt256 Px_mod_n = P_aff.x;
    if (cmp_UInt256(&Px_mod_n, &SECP256K1_N) >= 0) {
        sub_UInt256(&Px_mod_n, &SECP256K1_N);
    }

    // 比較
    if (cmp_UInt256(&Px_mod_n, &sig->r) == 0) {
        return 1; // Valid!
    } else {
        return 0; // Invalid
    }
}

// OSの乱数源(/dev/urandom)から256bitの乱数を取得する
// 成功したら 1, 失敗したら 0 を返す
int get_secure_random(UInt256* num) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (f == NULL) {
        fprintf(stderr, "Fatal Error: Failed to open /dev/urandom\n");
        return 0;
    }
    
    // uint64_t x 4 = 256bit 分読み込む
    size_t result = fread(num->limbs, sizeof(uint64_t), 4, f);
    fclose(f);

    if (result != 4) {
        fprintf(stderr, "Fatal Error: Failed to read enough entropy\n");
        return 0;
    }
    return 1;
}


// 鍵ペア生成
void generate_keypair(KeyPair* pair) {
    // 1. 秘密鍵 d の生成
    // 範囲 [1, N-1] に収まる乱数を探す
    do {
        if (!get_secure_random(&pair->private_key)) {
            // 乱数取得失敗時は強制終了（本来はエラーハンドリングすべき）
            return;
        }
        
        // 生成した乱数が N 以上、あるいは 0 ならやり直し
        // (厳密には 0 チェックも必要。is_zero_value を使う)
    } while (cmp_UInt256(&pair->private_key, &SECP256K1_N) >= 0 || is_zero_value(&pair->private_key));

    // 2. 公開鍵 Q = d * G の計算
    Point G_jac, Q_jac;
    init_point_G(&G_jac);
    
    // Gをモンゴメリ空間へ
    to_mont(&G_jac.x, &G_jac.x);
    to_mont(&G_jac.y, &G_jac.y);
    to_mont(&G_jac.z, &G_jac.z);

    // Q = d * G (スカラ倍算)
    // ここであなたの高速エンジンが火を噴く
    mul_scalar(&Q_jac, &G_jac, &pair->private_key);

    // 公開鍵は通常空間(Affine)で保持するのが一般的
    jacobian_to_affine(&pair->public_key, &Q_jac);
}