import os
import sys

try:
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
    from cryptography.hazmat.backends import default_backend
except ImportError:
    print("Error: 'cryptography' package is missing.")
    print("Please install it by running: pip install cryptography")
    sys.exit(1)

def encrypt_file(input_path, output_path, key_hex):
    if len(key_hex) != 64:
        print(f"Error: Key must be 64 hex characters (32 bytes). Got {len(key_hex)}")
        sys.exit(1)
        
    key = bytes.fromhex(key_hex)
    iv = os.urandom(16)
    
    cipher = Cipher(algorithms.AES(key), modes.CTR(iv), backend=default_backend())
    encryptor = cipher.encryptor()
    
    try:
        with open(input_path, 'rb') as f_in, open(output_path, 'wb') as f_out:
            f_out.write(iv)
            while True:
                chunk = f_in.read(4096)
                if not chunk:
                    break
                f_out.write(encryptor.update(chunk))
            f_out.write(encryptor.finalize())
        print(f"Success: Encrypted OTA binary saved to {output_path}")
    except Exception as e:
        print(f"Failed to encrypt: {e}")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python encrypt_ota.py <input.bin> <output.bin> <key_hex>")
        sys.exit(1)
    encrypt_file(sys.argv[1], sys.argv[2], sys.argv[3])
