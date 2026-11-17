#include "TextDocument.h"

#include <cctype>
#include <sstream>
#include <string>
#include <utility>

TextDocument::TextDocument(std::string text)
    : text_(std::move(text)) {
}

bool TextDocument::isWordCharacter(unsigned char character) {
    return std::isalnum(character) != 0
        || character >= 128
        || character == '_';
}

void TextDocument::capitalizeUtf8Character(
    std::string& text,
    std::size_t position
) {
    if (position >= text.size()) {
        return;
    }

    const unsigned char first =
        static_cast<unsigned char>(text[position]);

    if (first == 0xD1 && position + 1 < text.size()) {
        const unsigned char second =
            static_cast<unsigned char>(text[position + 1]);

        if (second == 0x91) {
            text[position] = static_cast<char>(0xD0);
            text[position + 1] = static_cast<char>(0x81);
            return;
        }

        if (second >= 0x80 && second <= 0x8F) {
            text[position] = static_cast<char>(0xD0);
            text[position + 1] =
                static_cast<char>(second - 0x20);
            return;
        }
    }

    if (first == 0xD0 && position + 1 < text.size()) {
        const unsigned char second =
            static_cast<unsigned char>(text[position + 1]);

        if (second >= 0xB0 && second <= 0xBF) {
            text[position + 1] =
                static_cast<char>(second - 0x20);
        }
    }

    if (first < 128) {
        text[position] = static_cast<char>(
            std::toupper(first)
        );
    }
}

std::string TextDocument::replaceWords(
    const std::string& oldWord,
    const std::string& newWord
) {
    if (oldWord.empty()) {
        return text_;
    }

    std::size_t position = 0;

    while ((position = text_.find(oldWord, position))
           != std::string::npos) {

        const bool leftBoundary =
            position == 0
            || !isWordCharacter(
                static_cast<unsigned char>(text_[position - 1])
            );

        const std::size_t end = position + oldWord.size();

        const bool rightBoundary =
            end >= text_.size()
            || !isWordCharacter(
                static_cast<unsigned char>(text_[end])
            );

        if (leftBoundary && rightBoundary) {
            text_.replace(position, oldWord.size(), newWord);
            position += newWord.size();
        } else {
            position += oldWord.size();
        }
    }

    return text_;
}

std::string TextDocument::searchPhrase(
    const std::string& phrase
) const {
    if (phrase.empty()) {
        return "Empty search phrase";
    }

    std::ostringstream result;
    std::size_t position = 0;
    bool found = false;

    while ((position = text_.find(phrase, position))
           != std::string::npos) {

        if (found) {
            result << ", ";
        }

        result << position;
        found = true;
        position += phrase.size();
    }

    if (!found) {
        return "Phrase not found";
    }

    return "Found at positions: " + result.str();
}

std::string TextDocument::capitalizeWords() {
    bool newWord = true;

    for (std::size_t i = 0; i < text_.size(); ++i) {
        const unsigned char character =
            static_cast<unsigned char>(text_[i]);

        if (isWordCharacter(character)) {
            if (newWord) {
                capitalizeUtf8Character(text_, i);
            }

            newWord = false;
        } else {
            newWord = true;
        }
    }

    return text_;
}

std::string TextDocument::capitalizeSentences() {
    bool sentenceStart = true;

    for (std::size_t i = 0; i < text_.size(); ++i) {
        const unsigned char character =
            static_cast<unsigned char>(text_[i]);

        if (sentenceStart && isWordCharacter(character)) {
            capitalizeUtf8Character(text_, i);
            sentenceStart = false;
        }

        if (character == '.' || character == '!'
            || character == '?') {
            sentenceStart = true;
        }
    }

    return text_;
}

std::string TextDocument::numberSentences() {
    std::ostringstream result;
    std::string sentence;
    std::size_t sentenceNumber = 1;

    for (std::size_t i = 0; i < text_.size(); ++i) {
        const char character = text_[i];
        sentence += character;

        const bool isEndMark =
            character == '.' || character == '!' || character == '?';

        const bool nextIsEndMark =
            i + 1 < text_.size()
            && (text_[i + 1] == '.'
                || text_[i + 1] == '!'
                || text_[i + 1] == '?');

        if (isEndMark && !nextIsEndMark) {
            std::size_t start = 0;

            while (start < sentence.size()
                   && std::isspace(
                       static_cast<unsigned char>(sentence[start])
                   ) != 0) {
                ++start;
            }

            if (start < sentence.size()) {
                if (sentenceNumber > 1) {
                    result << '\n';
                }

                result << sentenceNumber << ". "
                       << sentence.substr(start);

                ++sentenceNumber;
            }

            sentence.clear();
        }
    }

    std::size_t start = 0;

    while (start < sentence.size()
           && std::isspace(
               static_cast<unsigned char>(sentence[start])
           ) != 0) {
        ++start;
    }

    if (start < sentence.size()) {
        if (sentenceNumber > 1) {
            result << '\n';
        }

        result << sentenceNumber << ". "
               << sentence.substr(start);
    }

    return result.str();
}

const std::string& TextDocument::getText() const {
    return text_;
}
