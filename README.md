# raw-secp256k1
A lightweight, dependency-free scratch-build implementation of **secp256k1** Elliptic Curve Cryptography in pure C.

Designed for deep educational understanding of cryptographic primiteives, focusing on low level arithmetic optimization and system integration.

## 🚀 Key Features
- **No External Dependencies:** Built using only standards C libraries (`<stdint.h>`, `<stdlib.h>`, `<stdio.h>`). No OpenSSL, GMP, or other heaby dependencies.
- **Montgomry Multiplication (CIOS):** Implements the Coarsely Integrated Operand Scanning (CIOS) method for high-performance modular arithmetic.
- **Jacobian Coordinates:** Utilizes projective coordinates to eliminate expensive modular inversions during point addition and doubling.
- **System Integrity** Integrates with `/dev/urandom` for secure entropy retrieval during key generation.
- **Complete Lifecycle** Support Key Pair Generation ECDSA Signing, and Verification.

## 🛠 Project Structure

```text
raw-secp256k1/
├── include/       # Header files (declarations and constants)
├── src/           # Implementation (Finite fields, Group operations, ECDSA)
├── tests/         # Unit tests and integration tests
└── Makefile       # Automated build script
```

## 📦 Quick Start
### Prerequistes
- GCC or Clang
- Make
- Linux/macOS/WSL(for `/dev/urandom`)

### build and Run
This project includes a Makefile for easy compilation.

```Bash
# Build the main.c
make

# Build and run the tests immediately
make run

#Clean build artifacts
make clean
```

## 📖 Usage Example
Here is a simple example of generating a key pair, signing a message, and verifying the signature.

```C:main.c
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
```

## ⚠️ Disclaimer & Future Work
This library was created for educational purposes to understand thew mathematical underpinnings of ECC.

- **Side-Channel Attacks:** While the logic is mathematically correct the current implementations is **not fully constant-time**. Branching in `add_Jacobian` and other functions may leak infomation via timing or power analysis.
- **RFC 6079:** Deterministic nonce generation (RFC 6979) is not yet implemented. It currently relies on a secure RNG for k.

**DO NOT use this in production environments where assets or sensitive data are at risk.**

## 📄 License
This project is licensed under the MIT License