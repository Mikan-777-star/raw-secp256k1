#ifndef SECP256K1_UTILS_H
#define SECP256K1_UTILS_H
#include <stdint.h>
#include <stdio.h>
//unsigned int 256bits
typedef struct uint256{
    uint64_t limbs[4];
}UInt256;

extern const UInt256 SECP256K1_CORRECTION ;
extern const UInt256 SECP256K1_R ;
extern const UInt256 SECP256K1 ;

extern const UInt256 SECP256K1_N;

void printHex_UInt256(const UInt256* ptr);

// 128bit整数型（GCC/Clang用）
typedef unsigned __int128 uint128_t;

// SECP256K1 の法 P (定義済みだと思うけど、再確認)
// P = 0xFFFFFFFEFFFFFC2F...
// limbs[0] = 0xFFFFFFFEFFFFFC2F

// モンゴメリ定数 P_DASH ( -P^-1 mod 2^64 )
// これは「マジックナンバー」よ。
// 計算式: r * P_DASH == -1 (mod 2^64) となる r (Pの下位64bit) の逆数。
// SECP256K1の場合、計算するとこうなるわ。
extern const uint64_t P_DASH ;

// モンゴメリ変換用定数 R^2 mod P
// 計算式: (2^256 mod P)^2
extern const UInt256 SECP256K1_RR;

extern const UInt256 secp256k1_2 ;

typedef struct point {
    UInt256 x;
    UInt256 y;
    UInt256 z; // ヤコビアン座標用。無限遠点は z=0 で表現するのが一般的
} Point;

// secp256k1 Generator Point (Affien coordinates)
extern const UInt256 G_X ;
extern const UInt256 G_Y ;

// 簡易初期化関数
void init_point_G(Point* p) ;
typedef struct signature {
    UInt256 r;
    UInt256 s;
} Signature;

void zero_UInt256(UInt256* ptr);
void scalar_add(UInt256* res, const UInt256* A, const UInt256* B);

void scalar_sub(UInt256* res, const UInt256* A, const UInt256* B);
void scalar_mul(UInt256* res, const UInt256* A, const UInt256* B);
void scalar_pow(UInt256* res, const UInt256* base, const UInt256* exp);
void scalar_inv(UInt256* res, const UInt256* A);
/**
 * 署名生成
 * r, s : 結果の格納先
 * z    : メッセージハッシュ (32byte数値)
 * d    : 秘密鍵
 * k    : エフェメラル鍵 (ランダムな値。定数ではない！)
 */
int is_zero_value(const UInt256* t);
void ecdsa_sign(Signature* sig, const UInt256* z, const UInt256* d, const UInt256* k) ;
int ecdsa_verify(const Signature* sig, const UInt256* z, const Point* Q_aff);

// OSの乱数源(/dev/urandom)から256bitの乱数を取得する
// 成功したら 1, 失敗したら 0 を返す
int get_secure_random(UInt256* num) ;

typedef struct {
    UInt256 private_key; // d
    Point public_key;    // Q = dG (Affine)
} KeyPair;

// 鍵ペア生成
void generate_keypair(KeyPair* pair) ;
int is_zero(const Point* point);
void double_Jacobian(Point* result, const Point* A);
void set_inf(Point* ptr);
void add_Jacobian(Point* result, const Point* A,const Point* B);
void mul_scalar(Point* result, const Point* P, const UInt256* k);
void jacobian_to_affine(Point* affine_res, const Point* jacobian_pt);
void conditional_sub_p(UInt256* val, const UInt256* P) ;


/**
 * res: 結果
 * A: 引数1
 * B: 引数2
 * P: Mod
 */
void mod_add(UInt256* result, const UInt256* A, const UInt256* B);

void mod_sub(UInt256* result, const UInt256* A, const UInt256* B);
//モンゴメリーする
void mod_mul(UInt256* res, const UInt256* A, const UInt256* B);

void legacy_mod_mul(UInt256* res, const UInt256* A, const UInt256* B);

void mod_pow(UInt256* res,  const UInt256* base, const UInt256* exp);

void legacy_mod_pow(UInt256* res, const UInt256* base, const UInt256* exp);
void mod_inv(UInt256* res, const UInt256* A);
//Aに戻り値が入ります
uint64_t add_UInt256(UInt256* A,const  UInt256* B);
uint64_t sub_UInt256(UInt256* A,const UInt256* B);
int cmp_UInt256(const UInt256* A,const  UInt256* B);
void not_UInt256(UInt256* A);

int get_bit_UInt256(const UInt256* A, int i);
void mul_mont(UInt256* res, const UInt256* A, const UInt256* B);
void mul_mont_debug(UInt256* res, const UInt256* A, const UInt256* B);
void to_mont(UInt256* res, const UInt256* A);
void from_mont(UInt256* res, const UInt256* montA);

#endif