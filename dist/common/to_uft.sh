#!/bin/bash

# 循环遍历当前文件夹下的所有JSON文件
for file in *.json
do
    # 判断文件是否存在并且为普通文件
    if [ -f "$file" ]; then
        # 使用iconv命令将文件编码从ISO-8859-1转换为UTF-8
        iconv -f gbk -t UTF-8 "$file" > "${file%.json}_utf8.json"
        # 将转换后的文件重命名为原文件名
        mv "${file%.json}_utf8.json" "$file"
        echo "文件 $file 转换完成"
    fi
done
