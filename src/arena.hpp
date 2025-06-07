#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <new>

class ArenaAllocator
{
    public:
        explicit ArenaAllocator(const size_t max_num_bytes)
            : m_size(max_num_bytes)
            , m_buffer(new std::byte[max_num_bytes])
            , m_offset(m_buffer)
        {
        }

        // Disable copy semantics to avoid double deletion
        ArenaAllocator(const ArenaAllocator&) = delete;
        ArenaAllocator& operator=(const ArenaAllocator&) = delete;

        // Move constructor: transfer ownership of buffer and size
        ArenaAllocator(ArenaAllocator&& other) noexcept
            : m_size(std::exchange(other.m_size, 0))
            , m_buffer(std::exchange(other.m_buffer, nullptr))
            , m_offset(std::exchange(other.m_offset, nullptr))
        {
        }

        // Move assignment operator: swap resources safely
        ArenaAllocator& operator=(ArenaAllocator&& other) noexcept
        {
            std::swap(m_size, other.m_size);
            std::swap(m_buffer, other.m_buffer);
            std::swap(m_offset, other.m_offset);

            return *this;
        }

        // Allocate uninitialized memory for one object of type T, properly aligned
        template <typename T>
        [[nodiscard]] T* alloc()
        {
            size_t remaining_num_bytes = m_size - static_cast<size_t>(m_offset - m_buffer);

            void* ptr = m_offset;

            // Align pointer to alignment requirement of T and adjust remaining size
            void* aligned_address = std::align(alignof(T), sizeof(T), ptr, remaining_num_bytes);

            if (aligned_address == nullptr)
                throw std::bad_alloc{};

            // Move offset pointer past allocated space
            m_offset = static_cast<std::byte*>(aligned_address) + sizeof(T);

            return static_cast<T*>(aligned_address);
        }

        // Allocate and construct an object of type T with given arguments
        template <typename T, typename... Args>
        [[nodiscard]] T* emplace(Args&&... args)
        {
            T* memory = alloc<T>();
            
            return new (memory) T(std::forward<Args>(args)...);
        }

        ~ArenaAllocator()
        {
            // Note: destructors of stored objects are NOT called automatically.
            // Users must manually destroy objects if needed to avoid resource leaks.
            delete[] m_buffer;
        }
    
    private:
        size_t m_size;          // total size of buffer in bytes
        std::byte* m_buffer;    // start of the buffer
        std::byte* m_offset;    // current allocation offset pointer
};