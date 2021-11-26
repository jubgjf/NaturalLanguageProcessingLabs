def read_src(filename_list: list) -> dict:
    """读取语料库并生成词典，词典的格式为
    {
        "词": 词频,
        ...,
        "词": 词频,
    }

    Args:
        filename_list: 语料库文件路径

    Returns:
        返回词典 dict
    """

    dic: dict = {}
    for filename in filename_list:
        with open(filename, "r") as f:
            for line in f:
                if line == "\n":
                    # 空行，跳过
                    continue
                elif line.startswith("1998"):
                    word_list: list[str] = line.split(" ")  # 训练集的一行
                    word_list = word_list[1:-1]  # 去除时间和换行符
                    word_list = [i for i in word_list if i != ""]  # 去除用 "" 切分后剩下的空字符
                    for word in word_list:
                        # word_ps[0] -> 词
                        # word_ps[1] -> 词性
                        word_ps: list = word.split("/")
                        word = word_ps[0].replace("[", "").replace("]", "")  # 删除中括号

                        if word not in dic.keys():
                            dic[word] = 1
                        else:
                            dic[word] += 1
                else:
                    # 人名补充
                    word = line.strip()
                    if word not in dic.keys():
                        dic[word] = 1
                    else:
                        dic[word] += 1

    return dic


def write_dic(filename: str, dic: dict) -> None:
    """将词典 dict 写入到文件，文件的格式为
    词<Space>词频<CR>
    词<Space>词频<CR>
    ...
    词<Space>词频<CR>

    Args:
        filename: 文件路径
        dic:      词典
    """

    with open(filename, "w") as f:
        for item in dic.keys():
            f.write(item + " " + str(dic[item]) + "\n")


if __name__ == "__main__":
    dic = read_src(["lab1/dataset/199802_segpos.txt"])
    write_dic("lab1/dic/dic.txt", dic)
