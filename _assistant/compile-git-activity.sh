#!/bin/bash

# Usage   : ./compile-git-activity.sh <GitHub Repo URL> <Branch Name> <Start Date> <End Date>
# Example : ./compile-git-activity.sh https://github.com/dennisppaul/umfeld SDL3-dev "2025-03-01" "2025-03-04"

# Input validation
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <GitHub Repo URL> <Branch Name> <Start Date> <End Date>"
    exit 1
fi

REPO_URL="$1"
BRANCH_NAME="$2"
START_DATE="$3"
END_DATE="$4"
TEMP_DIR=$(mktemp -d)
OUTPUT_FILE="${END_DATE}-🤖-Summary-${START_DATE}>${END_DATE}(@$(date +%Y-%m-%d)--${BRANCH_NAME}).txt"
HEADER_FILE="${END_DATE}-🤖-Summary-${START_DATE}>${END_DATE}.md"

echo "---"                                               > "$HEADER_FILE"
echo "layout: post"                                     >> "$HEADER_FILE"
echo "title:  \"🤖 Summary ${START_DATE}>${END_DATE}\"" >> "$HEADER_FILE"
echo "date:   ${END_DATE} 10:00:00 +0100"               >> "$HEADER_FILE"
echo "---"                                              >> "$HEADER_FILE"
echo ""                                                 >> "$HEADER_FILE"

# Clone the repository (shallow clone for efficiency)
git clone --quiet --depth=100 --branch "$BRANCH_NAME" --single-branch "$REPO_URL" "$TEMP_DIR"
cd "$TEMP_DIR" || exit 1

# Collect commits within the date range
echo "Fetching commits from branch '$BRANCH_NAME' ($START_DATE to $END_DATE) in $REPO_URL..." > "$OUTPUT_FILE"

# Extract commit history with diffs
git log --since="$START_DATE" --until="$END_DATE" --pretty=format:"Commit: %H%nAuthor: %an%nDate: %ad%n%nMessage:%n%s%n%n%b%n---" --patch --no-color >> "$OUTPUT_FILE"

# Move output to the original directory
mv "$OUTPUT_FILE" "$OLDPWD/"
echo "Saved commit log to $OUTPUT_FILE"

# Cleanup
cd "$OLDPWD" || exit 1
rm -rf "$TEMP_DIR"
