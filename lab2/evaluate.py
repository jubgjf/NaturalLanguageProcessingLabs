def calc_precision(result_file: str, answer_file: str) -> float:
    """计算识别分词结果准确率（识别结果中正确的比例）

    Args:
        result_file: 识别结果文件路径
        answer_file: 标准识别结果文件路径

    Returns:
        返回准确率
    """

    # 统计识别结果中有多少个实体
    result_ner_count: int = 0
    with open(result_file, "r") as f:
        for line in f:
            if "B-" in line:
                result_ner_count += 1

    # 对比结果和标准文件
    result_label_list: list[str]
    answer_label_list: list[str]
    with open(result_file, "r") as f:
        result_label_list = f.read().split("\n")
        for i in range(0, len(result_label_list)):
            result_label_list[i] = result_label_list[i][2:]
    with open(answer_file, "r") as af:
        answer_label_list = af.read().split("\n")
        for i in range(0, len(answer_label_list)):
            answer_label_list[i] = answer_label_list[i][2:]

    ner_start_line: list[int] = []
    ner_end_line: list[int] = []
    line_num: int = 0
    with open(answer_file, "r") as af:
        for aline in af:
            if "B-" in aline:
                ner_start_line.append(line_num)
            elif "E-" in aline:
                ner_end_line.append(line_num)
            line_num += 1
        if len(ner_start_line) != len(ner_end_line):
            print("===== Error =====")
            return -1

    ner_match_count: int = 0
    for i in range(0, len(ner_start_line)):
        start_line_num: int = ner_start_line[i]
        end_line_num: int = ner_end_line[i]
        matched: bool = True
        for j in range(start_line_num, end_line_num + 1):
            if result_label_list[j] != answer_label_list[j]:
                matched = False
                break
        if matched:
            ner_match_count += 1

    return ner_match_count / result_ner_count


def calc_recall(result_file: str, answer_file: str) -> float:
    """计算识别结果召回率（识别结果找出了正确实体的多少）

    Args:
        result_file: 识别结果文件路径
        answer_file: 标准识别结果文件路径

    Returns:
        返回召回率
    """

    # 统计正确实体有多少
    answer_ner_count: int = 0
    with open(answer_file, "r") as af:
        for aline in af:
            if "B-" in aline:
                answer_ner_count += 1

    # 对比结果和标准文件
    result_label_list: list[str]
    answer_label_list: list[str]
    with open(result_file, "r") as f:
        result_label_list = f.read().split("\n")
        for i in range(0, len(result_label_list)):
            result_label_list[i] = result_label_list[i][2:]
    with open(answer_file, "r") as af:
        answer_label_list = af.read().split("\n")
        for i in range(0, len(answer_label_list)):
            answer_label_list[i] = answer_label_list[i][2:]

    ner_start_line: list[int] = []
    ner_end_line: list[int] = []
    line_num: int = 0
    with open(answer_file, "r") as af:
        for aline in af:
            if "B-" in aline:
                ner_start_line.append(line_num)
            elif "E-" in aline:
                ner_end_line.append(line_num)
            line_num += 1
        if len(ner_start_line) != len(ner_end_line):
            print("===== Error =====")
            return -1

    ner_match_count: int = 0
    for i in range(0, len(ner_start_line)):
        start_line_num: int = ner_start_line[i]
        end_line_num: int = ner_end_line[i]
        matched: bool = True
        for j in range(start_line_num, end_line_num + 1):
            if result_label_list[j] != answer_label_list[j]:
                matched = False
                break
        if matched:
            ner_match_count += 1

    return ner_match_count / answer_ner_count


def calc_f(precision: float, recall: float) -> float:
    """计算准确率和召回率的调和平均
    Args:
        precision: 准确率
        recall:    召回率
    Returns:
        返回准确率和召回率的调和平均
    """

    return (2 * precision * recall) / (precision + recall)


def evaluate(result_file: str, answer_file: str):
    p: float = calc_precision(result_file, answer_file)
    r: float = calc_recall(result_file, answer_file)
    f: float = calc_f(p, r)
    print("presision = {}, recall = {}, f = {}".format(p, r, f))


if __name__ == '__main__':
    print("HMM", end=" ")
    evaluate("ner_clue_data/dev_char.txt", "result-HMM.txt")
    print("ME ", end=" ")
    evaluate("ner_clue_data/dev_char.txt", "result-ME.txt")
