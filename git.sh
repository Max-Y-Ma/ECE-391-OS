#!/bin/bash

# Check for the correct number of arguments
if [ "$#" -eq 0 ]; then
    echo -e "\e[1;32mDumb Dumb! Do This!\e[0m"
    echo -e "\e[1;32mUsage: $0 '<commit_message>' '<optional_branch_name>'\e[0m"
    exit 1
fi

commit_message="$1"
branch_name="$2"

# Check if the branch exists
if [ -n "$2" ]; then
    if ! git rev-parse --verify "$branch_name" > /dev/null 2>&1; then
        echo -e "\e[1;32mERROR: Branch [$branch_name] does not exist.\e[0m"
        exit 1
    fi
fi

# Add all files except those with .img extension
git add --all
git reset "*.img"
git reset "*/test_filesys_img"
git reset "user_tests/*"

# Commit the changes
git commit -m "$commit_message"

# # Push the changes to the specified branch
if [ -n "$2" ]; then
    git push origin "$branch_name"
    echo -e "\e[1;32mChanges committed and pushed to branch '$branch_name'.\e[0m"
    echo -e "\e[1;32m~ Dead & Locked!\e[0m"
fi