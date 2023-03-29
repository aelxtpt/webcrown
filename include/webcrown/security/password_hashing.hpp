#pragma once
#include <cstdint>
#include <string>
#include <cassert>
#include <stdexcept>

/// Password hashing interface
///
/// Password hashing interface allows to generate strong password hash and salt
/// to protect plain passwords from database access attack and validate the given
/// password over the generated hash and salt
///
/// https://en.wikipedia.org/wiki/Key_derivation_function
class password_hashing
{
    size_t hash_length_;
    size_t salt_length_;
public:
    explicit password_hashing(size_t hash_length = 32, size_t salt_length = 32)
        : hash_length_(hash_length)
        , salt_length_(salt_length)
    {
        assert((hash_length_ >= 8) && "Hash length should be at least 8 bytes!");
        if (hash_length_ < 8)
            throw std::runtime_error("Hash length should be at least 8 bytes");

        assert((salt_length_ >= 8) && "Salt length should be at least 8 bytes");
        if (salt_length_ < 8)
            throw std::runtime_error("Salt length should be at least 8 bytes");
    }

    password_hashing(password_hashing const&) = default;
    password_hashing(password_hashing &&) = default;
    ~password_hashing() = default;

    password_hashing& operator=(password_hashing const&) = default;
    password_hashing& operator=(password_hashing &&) = default;

    /// Get the strong password hash length
    size_t hash_length() const noexcept { return hash_length_; }

    /// Get the unique password salt length
    size_t salt_length() const noexcept { return salt_length_; }

    /// Get the password hashing algorithm name
//    virtual std::string const& name() const = 0;

    /// Generate the unique password salt
    /// \return unique password salt
    virtual std::string generate_salt() const;

    /// Generate the strong password hash for the given user password and unique salt
    /// \param password User password
    /// \param salt Unique password salt
    /// \return strong password hash
    virtual std::string generate_hash(std::string_view password, std::string_view salt) const = 0;

    /// Generate the strong password hash and unique salt for the given user password
    /// \param password
    /// \return
    virtual std::pair<std::string, std::string> generate_hash_and_salt(std::string_view password) const;

    /// Generate the secure digest string for the given user password
    /// \param password User password
    /// \return Secure digest string
    virtual std::string generate_digest(std::string_view password) const;

    /// Generate the secure base64 digest string for the given user password
    /// \param password User password
    /// \return Secure base64 digest string
    virtual std::string generate_encoded_digest(std::string_view password) const;

    /// Validate the user password over the given strong password hash and unique salt
    /// \param password User password
    /// \param hash Strong password hash
    /// \param salt Unique password salt
    /// \return 'true' if the given user password is valid, 'false' if the given password is invalid
    virtual bool validate(std::string_view password, std::string_view hash, std::string_view salt) const = 0;

    /// Validate the user password over the given secure digest string
    /// \param password User password
    /// \param digest Secure digest string
    /// \return 'true' if the given user password is valid, 'false' if the given user password is invalid
    virtual bool validate_digest(std::string_view password, std::string digest) const;

    /// Validate the user password over the given secure base64 digest string
    /// \param password User password
    /// \param digest Secure base64 digest string
    /// \return 'true' if the given user password is valid, 'false' if the given user password is invalid
    virtual bool validate_encoded_digest(std::string_view password, std::string_view digest) const;
};

class argon2d_password_hashing : public password_hashing
{
    uint32_t iterations_num_;
    uint32_t memory_usage_num_;
    uint32_t degree_parallelism_;
public:
    /// Initialize Argon2 password hashing with required parameters
    /// \param hash_length strong password hash length (default is 32)
    /// \param salt_length unique password salt length (default is 32)
    /// \param t number of iterations (default is 3)
    /// \param m Memory usage in kilobytes (default is 512)
    /// \param p Degree of parallelism (default is 1)
    explicit argon2d_password_hashing(size_t hash_length = 32, size_t salt_length = 32, uint32_t t = 3, uint32_t m = 512,
                                      uint32_t p = 1);

    argon2d_password_hashing(argon2d_password_hashing const&) = default;
    argon2d_password_hashing(argon2d_password_hashing &&) = default;
    argon2d_password_hashing& operator=(argon2d_password_hashing const&) = default;
    argon2d_password_hashing& operator=(argon2d_password_hashing &&) = default;
    ~argon2d_password_hashing() = default;

    /// Get the number of iterations
    uint32_t iterations_num() const noexcept { return iterations_num_; }
    /// Memory usage in kilobytes
    uint32_t memory_usage_num() const noexcept { return memory_usage_num_; }
    /// Get the degree of parallelism
    uint32_t degree_parallelism() const noexcept { return degree_parallelism_; }

    std::string generate_hash(std::string_view password, std::string_view salt) const override;

    bool validate(std::string_view password, std::string_view hash, std::string_view salt) const override;
};