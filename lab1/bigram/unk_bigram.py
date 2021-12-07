import math


def gen_pdic(uni_dic: dict, bi_dic: dict, lam: float) -> dict:
    """将二元文法词典的词频部分转换为对数概率，使用二元线性插值进行平滑

    Args:
        uni_dic: 一元文法词典
        bi_dic:  二元文法词典
        lam:     二元线性插值系数

    Returns:
        二元文法概率词典
    """

    uni_total_count: int = 0
    for count in uni_dic.values():
        uni_total_count += count
    bi_total_count: int = 0
    for count in bi_dic.values():
        bi_total_count += count

    pdic: dict = {}
    for key in bi_dic.keys():
        bi_possibility: float = bi_dic[key] / bi_total_count
        p_bi_possibility: float = math.log(bi_possibility) if bi_possibility != 0 else -100
        uni_possibility: float = uni_dic[key.split("-")[0]] / uni_total_count if not key.startswith(">") else 1
        p_uni_possibility: float = math.log(uni_possibility) if uni_possibility != 0 else -100

        pdic[key] = lam * p_bi_possibility + (1 - lam) * p_uni_possibility

    return pdic


def gen_pred_dic(uni_dic: dict) -> dict:
    """根据原始词典生成前缀词典；
    例如，原始词典为：
    {
        "去": 100,
        "北京大学": 88,
        "北京": 79
    }
    则前缀词典为：
    {
        "去": 100,
        "北京大学": 88,
        "北京大":0
        "北":0
        "北京": 79
    }

    Args:
        uni_dic: 原始词典

    Returns:
        返回前缀词典

    """

    pred_dic: dict[str:int] = {}
    for word in uni_dic.keys():
        pred_dic[word] = uni_dic[word]
        for i in range(1, len(word)):
            if word[0:i] not in uni_dic.keys():
                pred_dic[word[0:i]] = 0

    return pred_dic


def read_dic(filename: str) -> dict:
    """从文件中读取一元或二元文法词典，词典格式为
    {
        "前缀词-后缀词": 词频,
        ...,
        "前缀词-后缀词": 词频,
    }
    其中 ">" 代表句首， "<" 代表句尾

    Args:
        filename: 语料库文件路径

    Returns:
        返回词典 dict
    """

    dic: dict[str:int] = {}
    with open(filename, "r") as f:
        for line in f:
            word_ps: list = line.strip().split(" ")
            dic[word_ps[0]] = int(word_ps[1])
    return dic


def bigram(uni_dic_filename: str, bi_dic_filename: str, sent_filename: str, seg_result_filename: str) -> None:
    """二元文法分词

    Args:
        uni_dic_filename:    一元文法字典文件路径
        bi_dic_filename:     二元文法字典文件路径
        sent_filename:       待分词文件路径
        seg_result_filename: 保存分词结果文件路径
    """

    # 一元文法原始词典
    uni_dic: dict[str:int] = read_dic(uni_dic_filename)

    # 一元文法前缀词典
    pred_uni_dic: dict[str:int] = gen_pred_dic(uni_dic)
    pred_uni_dic[">"] = 1  # 句首符号
    pred_uni_dic["<"] = 1  # 句尾符号

    # 二元文法原始词典
    bi_dic: dict[str:int] = read_dic(bi_dic_filename)

    # 二元文法概率词典
    lam: float = 0.75  # 平滑系数
    p_bi_dic: dict[str:float] = gen_pdic(uni_dic, bi_dic, lam)

    # 日志
    print("Read dic finished!")

    # 待分词句子
    with open(sent_filename, "r") as f:
        with open(seg_result_filename, "w") as wf:
            for line in f:
                if line == "\n":
                    wf.write("\n")
                    continue

                sentence: str = ">" + line.strip() + "<"

                # 句子对应的有向无环图
                graph: dict[int, list] = {}  # graph[i] 为节点 i 的后继节点
                r_graph: dict[int, list] = {}  # r_graph[i] 为节点 i 的前驱结点
                for i in range(0, len(sentence)):
                    graph[i] = []
                    r_graph[i + 1] = []
                for i in range(0, len(sentence)):
                    for j in range(i + 1, len(sentence) + 1):
                        if sentence[i:j] in pred_uni_dic.keys():
                            if pred_uni_dic[sentence[i:j]] != 0:
                                graph[i].append(j)
                                r_graph[j].append(i)
                        else:
                            break

                # 找最优路径
                r_route: dict[int, int] = {0: 0, 1: 0}
                for i in range(2, len(sentence) + 1):
                    if len(r_graph[i]) == 0:
                        # 没有前驱节点
                        # 将其前驱节点设置为上一个有前驱节点的节点
                        pred_index = i - 1
                        while pred_index > 0:
                            if len(r_graph[pred_index]) > 0:
                                break
                            pred_index -= 1
                        r_route[i] = pred_index
                    elif len(r_graph[i]) == 1:
                        # 只有一个前驱节点
                        # 直接选择
                        r_route[i] = r_graph[i][0]
                    else:
                        # 多个前驱节点
                        # 选择代价最小的
                        max_poss_index = -1
                        max_poss_cost = -math.inf
                        for pred_index in r_graph[i]:
                            bi_word: str = "-".join((sentence[r_route[pred_index]:pred_index], sentence[pred_index:i]))
                            cost: float = p_bi_dic[bi_word] if bi_word in p_bi_dic.keys() else -100
                            if max_poss_cost < cost:
                                max_poss_cost = cost
                                max_poss_index = pred_index
                        r_route[i] = max_poss_index

                # 分词结果
                seg_sentence = ""
                index = len(r_route) - 1
                single_word_start: int = -1
                single_word_end: int = -1
                while index > 0:
                    word: str = sentence[r_route[index]:index]

                    if len(word) == 1 and word != "<" and word != ">":
                        # 单个字成词，可能是未登录词
                        if single_word_end == -1:
                            single_word_end = index
                    else:
                        # 找到了未登录词的范围
                        if single_word_end != -1:
                            single_word_start = index
                            # 未登录词范围内的每一个相邻的组合都放入一元文法前缀词典和二元文法概率词典中
                            for i in range(single_word_end, single_word_start, -1):
                                pred: str = sentence[r_route[single_word_start]:single_word_start]
                                prefix: str = sentence[single_word_start:i]
                                bi_word = "-".join((pred, prefix))

                                if prefix not in pred_uni_dic.keys():
                                    pred_uni_dic[prefix] = 1
                                else:
                                    pred_uni_dic[prefix] += 1
                                if bi_word not in bi_dic.keys():
                                    bi_dic[bi_word] = 1
                                    p_bi_dic[bi_word] = -13
                                else:
                                    bi_dic[bi_word] += 1
                                    p_bi_dic[bi_word] += 0.01

                            single_word_start = -1
                            single_word_end = -1

                    seg_sentence = word + "/ " + seg_sentence
                    index = r_route[index]
                seg_sentence = seg_sentence[3:-3]  # 删除句首和句尾符号

                wf.write(seg_sentence + "\n")


if __name__ == "__main__":
    bigram("lab1/dic/dic.txt", "lab1/dic/bi_dic.txt", "lab1/dataset/199801_sent.txt",
           "lab1/seg_result/seg_LM_2_UNK.txt")
