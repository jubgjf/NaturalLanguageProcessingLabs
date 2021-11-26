import math


def gen_pdic(dic: dict) -> dict:
    """将词典的词频部分转换为对数概率
    若 dic 为： { "大学": 6, "北京": 10, "去": 4 }，
    则转换后的词典为： { "大学": log(0.3), "北京": log(0.5), "去": log(0.2) }

    Args:
        dic: 词典

    Returns:
        概率词典
    """
    total_count: int = 0
    for count in dic.values():
        total_count += count

    pdic: dict = {}
    for key in dic.keys():
        possibility: float = dic[key] / total_count
        if possibility == 0:
            pdic[key] = -100
        else:
            pdic[key] = math.log(possibility)

    return pdic


def gen_pred_dic(dic: dict) -> dict:
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
        dic: 原始词典

    Returns:
        返回前缀词典

    """

    pred_dic: dict[str:int] = {}
    for word in dic.keys():
        pred_dic[word] = dic[word]
        for i in range(1, len(word)):
            if word[0:i] not in dic.keys():
                pred_dic[word[0:i]] = 0

    return pred_dic


def find_route(index: int, pdic: dict, graph: dict, sentence: str) -> dict[int:int]:
    """从句子对应的有向无环图中找到最好的分词方式，返回一个 dict 表示分词方式，
    如句子：“去北京大学玩”，
    计算后得到的分词方式是： {1: 0, 2: 1, 3: 1, 4: 3, 5: 1, 6: 5}；
    其表示 0 去 1 北 2 京 3 大 4 学 5 玩 6
          |   |                  |    |
          +---+------------------+----+
    则分词为 去/北京大学/玩

    Args:
        index:    句子最后一个字对应的序号
        pdic:     词典，其中 key 为词，value 为概率
        graph:    句子对应的有向无环图
        sentence: 句子

    Returns:
        返回一个 dict，key 为当前节点序号，value 为上一节点序号
    """

    # 分词方式
    # 对 router() 相当于全局变量，可以看作是 router() 递归时的静态变量
    route: dict[int:int] = {}

    # 动态规划，记录 router 结果
    # key -> index, value -> cost
    router_result: dict[int:float] = {}

    def router(index: int, pdic: dict, graph: dict, sentence: str) -> float:
        """计算从句子开头到 index 的最大花费

        Args:
            index:    句子最后一个字对应的序号
            pdic:     词典，其中 key 为词，value 为概率
            graph:    句子对应的有向无环图
            sentence: 句子

        Returns:
            返回一个 dict，key 为当前节点序号，value 为上一节点序号
        """

        if index == 0:
            return 0

        # index 的前缀节点
        pred_index: list[int] = []
        for key in graph.keys():
            if index in graph[key]:
                pred_index.append(key)

        # pred_index 中各个节点到 index 的花费
        cost: dict[int:float] = {}
        for i in pred_index:
            if i in router_result.keys():
                r = router_result[i]
            else:
                r = router(i, pdic, graph, sentence)
                router_result[i] = r
            cost[i] = r + pdic[sentence[i:index]]

        # 找到最大的 value 对应的 key
        for i in cost.keys():
            if cost[i] == max(cost.values()):
                route[index] = i
                break

        if len(cost) == 0:
            # 当前的 index 没有前缀节点，所以代价可以看作无穷
            return -math.inf
        else:
            return max(cost.values())

    router(index, pdic, graph, sentence)
    return route


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

    dic: dict[str:int] = {}
    with open(filename, "r") as f:
        for line in f:
            word_ps: list = line.strip().split(" ")
            dic[word_ps[0]] = int(word_ps[1])
    return dic


def unigram(dic_filename: str, sent_filename: str, seg_result_filename: str) -> None:
    """一元文法分词

    Args:
        dic_filename:        字典文件路径
        sent_filename:       待分词文件路径
        seg_result_filename: 保存分词结果文件路径
    """

    # 原始词典
    dic: dict[str:int] = read_dic(dic_filename)

    # 概率词典
    pdic: dict[str:float] = gen_pdic(dic)

    # 前缀词典
    pred_dic: dict[str:int] = gen_pred_dic(dic)

    # 待分词句子
    with open(sent_filename, "r") as f:
        with open(seg_result_filename, "w") as wf:
            for line in f:
                if line == "\n":
                    wf.write("\n")
                    continue

                sentence: str = line.strip()

                # 句子对应的有向无环图
                graph: dict = {}
                for i in range(0, len(sentence)):
                    graph[i] = []
                for i in range(0, len(sentence)):
                    for j in range(i + 1, len(sentence) + 1):
                        if sentence[i:j] in pred_dic.keys():
                            if pred_dic[sentence[i:j]] != 0:
                                graph[i].append(j)
                        else:
                            break

                # 分词方式
                route = find_route(len(sentence), pdic, graph, sentence)

                # 分词后的句子
                seg_sentence: str = ""
                curr_index = max(route.keys())
                while True:
                    seg_sentence = sentence[route[curr_index]:curr_index] + "/ " + seg_sentence
                    curr_index = route[curr_index]
                    if curr_index == 0:
                        break

                wf.write(seg_sentence + "\n")


if __name__ == "__main__":
    unigram("lab1/dic/dic.txt", "lab1/dataset/199801_sent.txt", "lab1/seg_result/seg_LM.txt")
