#!/bin/bash

# Integration test for syngen

BINARY=./bin/syngen
OUTPUT_ZIP="test_integration.zip"
EXTRACT_DIR="test_integration_extract"

# Clean up
rm -f $OUTPUT_ZIP
rm -rf $EXTRACT_DIR

# 1. Run syngen
echo "Running syngen..."
$BINARY -c 5 -m 100 -u 8 $OUTPUT_ZIP
if [ $? -ne 0 ]; then
    echo "Error: syngen execution failed."
    exit 1
fi

if [ ! -f $OUTPUT_ZIP ]; then
    echo "Error: Output zip file not found."
    exit 1
fi

# 2. Extract
echo "Extracting zip..."
unzip -q $OUTPUT_ZIP -d $EXTRACT_DIR
if [ $? -ne 0 ]; then
    echo "Error: Unzip failed."
    exit 1
fi

# 3. Validation
echo "Verifying contents..."

# Check users.json
if [ ! -f "$EXTRACT_DIR/users.json" ]; then
    echo "Error: users.json missing."
    exit 1
fi

# Check channels.json
if [ ! -f "$EXTRACT_DIR/channels.json" ]; then
    echo "Error: channels.json missing."
    exit 1
fi

# Basic content check (grep for ID pattern)
# cJSON formats with whitespace, e.g. "id": "U..."
USER_COUNT=$(grep -c "\"id\":[[:space:]]*\"U" "$EXTRACT_DIR/users.json")
CHANNEL_COUNT=$(grep -c "\"id\":[[:space:]]*\"C" "$EXTRACT_DIR/channels.json")

echo "Found $USER_COUNT users (expected 8)"
echo "Found $CHANNEL_COUNT channels (expected 5)"

if [ "$USER_COUNT" -ne 8 ]; then
    echo "Error: User count mismatch."
    exit 1
fi

if [ "$CHANNEL_COUNT" -ne 5 ]; then
    echo "Error: Channel count mismatch."
    exit 1
fi

echo "Integration test passed!"

# Clean up
rm -f $OUTPUT_ZIP
rm -rf $EXTRACT_DIR
