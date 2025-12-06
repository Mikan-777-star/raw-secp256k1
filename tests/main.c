#include "secp256k1_utils.h"
#include <stdio.h>

int main() {
    // 1. Generate Key Pair
    KeyPair pair;
    generate_keypair(&pair);
    printf("Keys generated.\n");

    // 2. Prepare Message Hash (32 bytes)
    UInt256 msg_hash;
    get_secure_random(&msg_hash); // Random hash for demo

    // 3. Prepare Nonce k (Must be random and secret!)
    UInt256 k;
    get_secure_random(&k);

    // 4. Sign
    Signature sig;
    ecdsa_sign(&sig, &msg_hash, &pair.private_key, &k);
    printf("Message signed.\n");

    // 5. Verify
    if (ecdsa_verify(&sig, &msg_hash, &pair.public_key)) {
        printf("Signature Valid!\n");
    } else {
        printf("Signature Invalid.\n");
    }

    return 0;
}