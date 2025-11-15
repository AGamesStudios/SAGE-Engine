#pragma once

#include "UTF8Utils.h"

#include <cstdint>
#include <iterator>
#include <string>

namespace SAGE::Core {

namespace utf8 {

using String = std::string;
using StringView = std::string_view;

class UTF8String {
public:
    UTF8String() = default;
    UTF8String(const char* str) : data_(str ? str : "") {}
    UTF8String(const char8_t* str)
        : data_(str ? reinterpret_cast<const char*>(str) : "") {}
    UTF8String(const std::u8string& str)
        : data_(reinterpret_cast<const char*>(str.c_str()), str.size()) {}
    UTF8String(std::string str) : data_(std::move(str)) {}

    size_t Length() const {
        return UTF8Utils::CountCodePoints(data_);
    }

    UTF8String Substr(size_t start, size_t length) const {
        return UTF8String(UTF8Utils::Substring(data_, start, length));
    }

    bool Contains(const UTF8String& needle) const {
        return UTF8Utils::Find(data_, needle.data_) != UTF8Utils::npos;
    }

    const std::string& ToStdString() const {
        return data_;
    }

    const char* c_str() const {
        return data_.c_str();
    }

    bool empty() const {
        return data_.empty();
    }

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = uint32_t;
        using difference_type = std::ptrdiff_t;
        using pointer = const uint32_t*;
        using reference = uint32_t;

        Iterator() = default;

        explicit Iterator(const std::string* source) : source_(source) {
            if (source_) {
                Load();
            }
        }

        reference operator*() const {
            return current_;
        }

        Iterator& operator++() {
            Advance();
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            Advance();
            return temp;
        }

        bool operator==(const Iterator& other) const {
            return source_ == other.source_ && byteOffset_ == other.byteOffset_;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

    private:
        void Advance() {
            if (!source_) {
                return;
            }
            if (nextOffset_ >= source_->size()) {
                source_ = nullptr;
                byteOffset_ = 0;
                nextOffset_ = 0;
                current_ = 0;
                return;
            }

            byteOffset_ = nextOffset_;
            size_t offset = byteOffset_;
            if (!UTF8Utils::NextCodePoint(*source_, offset, current_)) {
                source_ = nullptr;
                byteOffset_ = 0;
                nextOffset_ = 0;
                current_ = 0;
                return;
            }
            nextOffset_ = offset;
        }

        void Load() {
            if (!source_ || source_->empty()) {
                source_ = nullptr;
                byteOffset_ = 0;
                nextOffset_ = 0;
                current_ = 0;
                return;
            }

            byteOffset_ = 0;
            size_t offset = 0;
            if (!UTF8Utils::NextCodePoint(*source_, offset, current_)) {
                source_ = nullptr;
                byteOffset_ = 0;
                nextOffset_ = 0;
                current_ = 0;
                return;
            }
            nextOffset_ = offset;
        }

        const std::string* source_ = nullptr;
        size_t byteOffset_ = 0;
        size_t nextOffset_ = 0;
        uint32_t current_ = 0;
    };

    Iterator begin() const {
        return Iterator(data_.empty() ? nullptr : &data_);
    }

    Iterator end() const {
        return Iterator();
    }

private:
    std::string data_;
};

} // namespace utf8

} // namespace SAGE::Core
