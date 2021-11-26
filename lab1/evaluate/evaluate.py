def get_word_seg(seg_file: str, seg_ans_file: str) -> tuple[list, list]:
    """获取待评价分词结果文件和标准分词文件中的所有词

    Args:
        seg_file:     分词结果文件路径
        seg_ans_file: 标准分词结果文件路径

    Returns:
        返回 ( 分词结果的所有行列表, 标准分词结果的所有行列表 )
    """

    # 分词文件的所有行
    seg: list = []
    with open(seg_file, "r") as f:
        for line in f:
            if line == "\n":
                continue
            # 标准分词的一行
            word_list: list = line.split(" ")
            if word_list[-1] == "\n":
                # 去除换行符
                word_list = word_list[:-1]
            for i in range(0, len(word_list)):
                word_list[i] = word_list[i].split("/")[0]  # 去除词性
            seg.append(word_list)

    # 标准分词文件的所有行
    seg_ans: list = []
    with open(seg_ans_file, "r") as f:
        for line in f:
            if line == "\n":
                continue
            # 标准分词的一行
            word_list: list = line.split(" ")
            # 去除空格，处理标准分词文件中词之间空格数量不确定的情况
            new_word_list: list = [i for i in word_list if i != ""]
            word_list = new_word_list

            if word_list[-1] == "\n":
                # 去除换行符
                word_list = word_list[:-1]
            for i in range(0, len(word_list)):
                word_list[i] = (
                    word_list[i].split("/")[0].replace("[", "").replace("]", "")
                )  # 去除词性和中括号
            seg_ans.append(word_list)

    return seg, seg_ans


def calc_precision(seg_file: str, seg_ans_file: str) -> float:
    """计算分词结果准确率（分词结果中正确的比例）

    Args:
        seg_file:     分词结果文件路径
        seg_ans_file: 标准分词结果文件路径

    Returns:
        返回准确率
    """

    seg, seg_ans = get_word_seg(seg_file, seg_ans_file)

    # 分词结果中正确的个数
    right_count: int = 0

    # 分词后的词总数
    word_count: int = 0

    for line_index in range(0, len(seg)):
        word_count += len(seg[line_index])

        seg_line_word_index: int = 0
        seg_ans_line_word_index: int = 0
        while True:
            if seg_line_word_index >= len(seg[line_index]) or seg_ans_line_word_index >= len(seg_ans[line_index]):
                break

            if seg[line_index][seg_line_word_index] == seg_ans[line_index][seg_ans_line_word_index]:
                # 分词正确
                right_count += 1
                seg_line_word_index += 1
                seg_ans_line_word_index += 1
            else:
                # 分词不正确
                if len(seg[line_index][seg_line_word_index]) > len(seg_ans[line_index][seg_ans_line_word_index]):
                    # 第一种情况：我们在 seg_index 未切分，而标准分词切分了

                    # 调试日志
                    # print("Error 1 in line {}".format(line_index))
                    # print(seg[line_index][seg_line_word_index])
                    # print(seg_ans[line_index][seg_ans_line_word_index])

                    # 看连续的几个词能否匹配上
                    is_break = False
                    for k in range(1, len(seg[line_index]) - seg_line_word_index + 1):
                        for i in range(1, len(seg_ans[line_index]) - seg_ans_line_word_index + 1):
                            if "".join(
                                    seg[line_index][seg_line_word_index: seg_line_word_index + k]
                            ) == "".join(
                                seg_ans[line_index][seg_ans_line_word_index: seg_ans_line_word_index + i]
                            ):
                                seg_line_word_index += k
                                seg_ans_line_word_index += i
                                is_break = True
                                break
                        if is_break:
                            break
                    if not is_break:
                        print("Error: Code should not react here!")
                        return -1
                else:
                    # 第二种情况：标准分词在 seg_ans_index 未切分，而我们切分了

                    # 调试日志
                    # print("Error 2 in line {}".format(line_index))
                    # print(seg[line_index][seg_line_word_index])
                    # print(seg_ans[line_index][seg_ans_line_word_index])

                    # 看连续的几个词能否匹配上
                    is_break = False
                    for i in range(1, len(seg_ans[line_index]) - seg_ans_line_word_index + 1):
                        for k in range(1, len(seg[line_index]) - seg_line_word_index + 1):
                            if "".join(
                                    seg[line_index][seg_line_word_index: seg_line_word_index + k]
                            ) == "".join(
                                seg_ans[line_index][seg_ans_line_word_index: seg_ans_line_word_index + i]
                            ):
                                seg_line_word_index += k
                                seg_ans_line_word_index += i
                                is_break = True
                                break
                        if is_break:
                            break
                    if not is_break:
                        print("Error: Code should not react here!")
                        return -1

    return right_count / word_count


def calc_recall(seg_file: str, seg_ans_file: str) -> float:
    """计算分词结果召回率（分词结果找出了正确分词的多少）

    Args:
        seg_file:     分词结果文件路径
        seg_ans_file: 标准分词结果文件路径

    Returns:
        返回召回率
    """

    seg, seg_ans = get_word_seg(seg_file, seg_ans_file)

    # 正确结果中，分词的个数
    right_count: int = 0

    # 正确结果的词总数
    word_count: int = 0

    for line_index in range(0, len(seg)):
        word_count += len(seg_ans[line_index])

        seg_line_word_index: int = 0
        seg_ans_line_word_index: int = 0
        while True:
            if seg_line_word_index >= len(seg[line_index]) or seg_ans_line_word_index >= len(seg_ans[line_index]):
                break

            if seg[line_index][seg_line_word_index] == seg_ans[line_index][seg_ans_line_word_index]:
                # 分词正确
                right_count += 1
                seg_line_word_index += 1
                seg_ans_line_word_index += 1
            else:
                # 分词不正确
                if len(seg[line_index][seg_line_word_index]) > len(seg_ans[line_index][seg_ans_line_word_index]):
                    # 第一种情况：我们在 seg_index 未切分，而标准分词切分了

                    # 调试日志
                    # print("Error 1 in line {}".format(line_index))
                    # print(seg[line_index][seg_line_word_index])
                    # print(seg_ans[line_index][seg_ans_line_word_index])

                    # 看连续的几个词能否匹配上
                    is_break = False
                    for k in range(1, len(seg[line_index]) - seg_line_word_index + 1):
                        for i in range(1, len(seg_ans[line_index]) - seg_ans_line_word_index + 1):
                            if "".join(
                                    seg[line_index][seg_line_word_index: seg_line_word_index + k]
                            ) == "".join(
                                seg_ans[line_index][seg_ans_line_word_index: seg_ans_line_word_index + i]
                            ):
                                seg_line_word_index += k
                                seg_ans_line_word_index += i
                                is_break = True
                                break
                        if is_break:
                            break
                    if not is_break:
                        print("Error: Code should not react here!")
                        return -1
                else:
                    # 第二种情况：标准分词在 seg_ans_index 未切分，而我们切分了

                    # 调试日志
                    # print("Error 2 in line {}".format(line_index))
                    # print(seg[line_index][seg_line_word_index])
                    # print(seg_ans[line_index][seg_ans_line_word_index])

                    # 看连续的几个词能否匹配上
                    is_break = False
                    for i in range(1, len(seg_ans[line_index]) - seg_ans_line_word_index + 1):
                        for k in range(1, len(seg[line_index]) - seg_line_word_index + 1):
                            if "".join(
                                    seg[line_index][seg_line_word_index: seg_line_word_index + k]
                            ) == "".join(
                                seg_ans[line_index][seg_ans_line_word_index: seg_ans_line_word_index + i]
                            ):
                                seg_line_word_index += k
                                seg_ans_line_word_index += i
                                is_break = True
                                break
                        if is_break:
                            break
                    if not is_break:
                        print("Error: Code should not react here!")
                        return -1

    return right_count / word_count


def calc_f(precision: float, recall: float) -> float:
    """计算准确率和召回率的调和平均

    Args:
        precision: 准确率
        recall:    召回率

    Returns:
        返回准确率和召回率的调和平均
    """

    return (2 * precision * recall) / (precision + recall)


if __name__ == "__main__":
    seg_ans_file: str = "lab1/dataset/199801_segpos.txt"

    with open("lab1/evaluate/score.txt", "w") as f:
        # ===== FMM =====
        seg_file: str = "lab1/seg_result/seg_FMM_fast.txt"
        precision: float = calc_precision(seg_file, seg_ans_file)
        recall: float = calc_recall(seg_file, seg_ans_file)
        F: float = calc_f(precision, recall)

        f.write("===== FMM =====\n")
        f.write("precision = " + str(precision) + "\n")
        f.write("recall    = " + str(recall) + "\n")
        f.write("F         = " + str(F) + "\n")

        # ===== BMM =====
        seg_file: str = "lab1/seg_result/seg_BMM_fast.txt"
        precision: float = calc_precision(seg_file, seg_ans_file)
        recall: float = calc_recall(seg_file, seg_ans_file)
        F: float = calc_f(precision, recall)

        f.write("===== BMM =====\n")
        f.write("precision = " + str(precision) + "\n")
        f.write("recall    = " + str(recall) + "\n")
        f.write("F         = " + str(F) + "\n")
