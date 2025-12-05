#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "secp256k1_utils.h"

// ==========================================
// メインテスト
// ==========================================
int main() {
    printf("=== Full ECC Cycle Test (KeyGen -> Sign -> Verify) ===\n");

    // 1. 鍵ペア生成
    KeyPair pair;
    printf("[1] Generating Key Pair... ");
    generate_keypair(&pair);
    printf("Done.\n");

    printf("    Private Key: "); printHex_UInt256(&pair.private_key);
    printf("    Public Key X: "); printHex_UInt256(&pair.public_key.x);
    printf("    Public Key Y: "); printHex_UInt256(&pair.public_key.y);

    // 2. 署名対象のメッセージハッシュ (今回はランダムに作る)
    UInt256 z;
    get_secure_random(&z);
    printf("[2] Message Hash (z): "); printHex_UInt256(&z);

    // 3. 署名生成
    // 注意: 本来 k も乱数であるべき。
    // 今回はテスト用に別の乱数を取得して k とする
    UInt256 k;
    do {
        get_secure_random(&k);
    } while (cmp_UInt256(&k, &SECP256K1_N) >= 0 || is_zero_value(&k));

    Signature sig;
    printf("[3] Signing... ");
    ecdsa_sign(&sig, &z, &pair.private_key, &k);
    printf("Done.\n");
    printf("    Signature r: "); printHex_UInt256(&sig.r);
    printf("    Signature s: "); printHex_UInt256(&sig.s);

    // 4. 署名検証
    printf("[4] Verifying... ");
    int valid = ecdsa_verify(&sig, &z, &pair.public_key);

    if (valid) {
        printf("SUCCESS! The signature is VALID.\n");
    } else {
        printf("FAIL! The signature is INVALID.\n");
    }
    return 0;

}