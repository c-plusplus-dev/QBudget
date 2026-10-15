//
// Created by Ayaan on 2026-10-15.
//

#include "Block.h"
#include "Reader.h"
#include "Writer.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

int main() {
    try {
        const std::filesystem::path path = "block_test.qbdb";

        // Create test data.
        Block original;
        original.name = "Monthly expenses";

        original.items.push_back({"Food", 450});
        original.items.push_back({"Transport", 125});
        original.items.push_back({"Games", 600});
        original.items.push_back({"Savings", -200});

        std::cout << "Original block: " << original.name << '\n';
        std::cout << "Original items: " << original.items.size() << '\n';
        std::cout << "Original total: " << original.total() << "\n\n";

        // Serialize the block and save it to disk.
        Writer writer;
        original.writeTo(writer);
        writer.save(path);

        std::cout << "Saved to: "
                  << std::filesystem::absolute(path).string()
                  << '\n';

        // Load the saved file and deserialize the block.
        Reader reader(path);
        Block loaded = Block::readFrom(reader);

        // Verify the deserialized data.
        if (loaded.name != original.name) {
            throw std::runtime_error("Block name does not match");
        }

        if (loaded.items.size() != original.items.size()) {
            throw std::runtime_error("Item count does not match");
        }

        for (std::size_t i = 0; i < original.items.size(); ++i) {
            const auto& expected = original.items[i];
            const auto& actual = loaded.items[i];

            if (actual.name != expected.name) {
                throw std::runtime_error(
                    "Item name does not match at index "
                    + std::to_string(i)
                );
            }

            if (actual.cost != expected.cost) {
                throw std::runtime_error(
                    "Item cost does not match at index "
                    + std::to_string(i)
                );
            }
        }

        if (loaded.total() != original.total()) {
            throw std::runtime_error("Block total does not match");
        }

        if (!reader.atEnd()) {
            throw std::runtime_error(
                "Unexpected trailing bytes after the block"
            );
        }

        // Display the loaded data.
        std::cout << "\nLoaded block: " << loaded.name << '\n';
        std::cout << "Loaded items:\n";

        for (const auto& item : loaded.items) {
            std::cout << "  " << item.name
                      << ": " << item.cost << '\n';
        }

        std::cout << "Loaded total: " << loaded.total() << "\n\n";
        std::cout << "PASS: Block saving and loading work correctly.\n";

        return 0;
    } catch (const std::exception& exception) {
        std::cerr << "FAIL: " << exception.what() << '\n';
        return 1;
    }
}
