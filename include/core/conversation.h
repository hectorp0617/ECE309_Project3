#ifndef conversation_h
#define conversation_h

#include <cstddef>
#include "core/message.h"

#include <stdexcept>
#include <utility> // For std::swap

class Conversation {
public:

    // Default constructor initializes an empty conversation
    Conversation() = default;

    // Copy constructor to create a new Conversation as a copy of another
    Conversation(const Conversation& other) {
    if (other.size_ == 0) {
        return;
    }

    data_ = new Message[other.size_];

    try {
        for (std::size_t i = 0; i < other.size_; ++i) {
            data_[i] = other.data_[i];
        }
    } catch (...) {
        delete[] data_;
        throw;
    }

    size_ = other.size_;
    capacity_ = other.size_;
}
    //
    Conversation& operator=(const Conversation& other) {
        
        // Check for self-assignment
        if (this == &other) {
            return *this; // Handle self-assignment
        }
        
        Conversation replacement(other); // Create a copy of the other conversation
        std::swap(data_, replacement.data_); // Swap the data pointers
        std::swap(size_, replacement.size_); // Swap the sizes
        std::swap(capacity_, replacement.capacity_); // Swap the capacities


        return *this;
    }


    // Move constructor to transfer ownership of resources from another Conversation
    Conversation(Conversation&& other) noexcept {
        
        data_ = other.data_; // Transfer ownership of the data pointer
        size_ = other.size_; // Transfer the size
        capacity_ = other.capacity_; // Transfer the capacity

        // Reset the moved-from object to a valid state
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Conversation& operator=(Conversation&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    delete[] data_;

    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;

    return *this;
}


    // Destructor to clean up allocated memory
    ~Conversation() {
        delete[] data_;
    }

    void append(Message m) {
        if (size_ == capacity_) {
            // Handle capacity overflow (e.g., reallocate with increased capacity)
            std::size_t new_capacity;

            if (capacity_ == 0) {
                new_capacity = 1; // Start with a capacity of 1
            } else {
                new_capacity = capacity_ * 2; // Double the capacity
            }

            Message* new_data = new Message[new_capacity];

            try {
                for (std::size_t i = 0; i < size_; ++i) {
                    new_data[i] = data_[i]; // Copy existing messages to new array
                }
            } catch (...) {
                delete[] new_data; // Clean up if an exception occurs
                throw; // Rethrow the exception
            }

            delete[] data_; // Free the old array
            data_ = new_data; // Update data_ to point to the new array
            capacity_ = new_capacity; // Update capacity_
        }

        data_[size_] = m; // Add the new message to the end of the conversation
        ++size_; // Increment the size of the conversation
    }

    std::size_t size() const noexcept {
        // Return the number of messages in the conversation
        return size_;
    }

    // Boundary check for accessing messages in the conversation
    const Message& at(std::size_t i) const {
        // Return the message at index i
        if (i >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[i];
    }

    const Message* begin() const noexcept {
        // Return a pointer to the first message in the conversation
        return data_;
    }

    const Message* end() const noexcept {
        if (size_ == 0) {
            return data_;
        }
        // Return a pointer to one past the last message in the conversation
        return data_ + size_;
    }

private:
    Message* data_ = nullptr;
    std::size_t size_ = 0;      // Number of messages in the conversation
    std::size_t capacity_ = 0;      // Capacity of slots allocated for messages in the conversation

    friend struct ConversationTest; // Grant access to the test struct for testing purposes

    
};


#endif  // conversation_h