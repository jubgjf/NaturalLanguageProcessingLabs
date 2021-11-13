def read_src(filename: str) -> dict:
    """读取语料库并生成词典，词典的格式为
    {
        "词": 词频,
        ...,
        "词": 词频,
    }

    Args:
        filename: 语料库文件路径

    Returns:
        返回词典 dict
    """

    dic: dict = {}
    with open(filename, "r") as f:
        for line in f:
            word_list: list = line.split("  ")  # 训练集的一行
            word_list = word_list[1:-1]  # 去除时间和换行符
            for word in word_list:
                # word_ps[0] -> 词
                # word_ps[1] -> 词性
                word_ps: list = word.split("/")
                word = word_ps[0].replace("[", "").replace("]", "")  # 删除中括号

                if word not in dic.keys():
                    dic[word] = 1
                else:
                    dic[word] += 1
    return dic


def read_dic(filename: str) -> dict:
    """从文件中读取词典，词典格式为
    {
        "词": 词频,
        ...,
        "词": 词频,
    }

    Args:
        filename: 语料库文件路径

    Returns:
        返回词典 dict
    """

    dic: dict = {}
    with open(filename, "r") as f:
        for line in f:
            word_ps: list = line.strip().split(" ")
            dic[word_ps[0]] = int(word_ps[1])
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
    dic = read_src("lab1/dataset/199801_sent.txt")
    write_dic("lab1/dic/dic.txt", dic)
