from sklearn.linear_model import LogisticRegression
from sklearn.feature_extraction import DictVectorizer


def data_build(file_name: str, make_vocab=True):
    word_lists = []
    tag_lists = []
    with open(file_name, "r", encoding="utf-8") as file_read:
        word_list = []
        tag_list = []
        for line in file_read:
            if line != "\n":
                word, tag = line.strip("\n").split()
                word_list.append(word)
                tag_list.append(tag)
            else:
                word_lists.append(word_list)
                tag_lists.append(tag_list)
                word_list = []
                tag_list = []

    if make_vocab:
        word2id = {}
        for word_list in word_lists:
            for word in word_list:
                if word not in word2id:
                    word2id[word] = len(word2id)
        tag2id = {}
        for tag_list in tag_lists:
            for tag in tag_list:
                if tag not in tag2id:
                    tag2id[tag] = len(tag2id)
        return word_lists, tag_lists, word2id, tag2id
    return word_lists, tag_lists


def word2features(sent, i):
    """抽取单个字的特征"""
    word = sent[i]
    prev_word = "<s>" if i == 0 else sent[i - 1]
    next_word = "</s>" if i == (len(sent) - 1) else sent[i + 1]
    # 使用的特征：
    # 前一个词，当前词，后一个词，
    # 前一个词+当前词， 当前词+后一个词
    features = {
        'w': word,
        'w-1': prev_word,
        'w+1': next_word,
        'w-1:w': prev_word + word,
        'w:w+1': word + next_word,
        'w-1:w:w+1': prev_word + word + next_word,
        'bias': 1
    }
    return features


def sent2features(sent):
    """抽取序列特征"""
    return [word2features(sent, i) for i in range(len(sent))]


class MEModel:
    def __init__(self):
        self.model = LogisticRegression(penalty="l2", C=1.0)

    def train(self, train_word_lists, train_tag_lists, test_word_lists, test_tag_lists, test_word2id):
        all_train_words = []
        for t in train_word_lists:
            for tt in t:
                all_train_words.append(tt)

        test_word_lists_copy = []
        for i in range(0, len(test_word_lists)):
            r = []
            for j in range(0, len(test_word_lists[i])):
                if test_word_lists[i][j] in all_train_words:
                    r.append(test_word_lists[i][j])
                else:
                    r.append(test_word_lists[i][j] + str(test_word2id[test_word_lists[i][j]]))
            test_word_lists_copy.append(r)
        test_tag_lists_copy = []
        for i in range(0, len(test_tag_lists)):
            r = ["O"] * len(test_tag_lists[i])
            test_tag_lists_copy.append(r)

        word_lists = train_word_lists + test_word_lists_copy
        features = [sent2features(s) for s in word_lists]

        all_features = []
        for f in features:
            for ff in f:
                all_features.append(ff)
        v = DictVectorizer(sparse=True)
        X = v.fit_transform(all_features)

        all_tags = []
        tag_lists = train_tag_lists + test_tag_lists
        for t in tag_lists:
            for tt in t:
                all_tags.append(tt)
        self.model.fit(X, all_tags)

    def test(self, train_word_lists, test_word_lists):
        word_lists = train_word_lists + test_word_lists
        features = [sent2features(s) for s in word_lists]

        all_features = []
        for f in features:
            for ff in f:
                all_features.append(ff)
        v = DictVectorizer(sparse=True)
        X = v.fit_transform(all_features)

        pred_tag_lists = self.model.predict(X)

        all_words = []
        train_word_count = 0
        for t in train_word_lists:
            for tt in t:
                all_words.append(tt)
                train_word_count += 1
        for t in test_word_lists:
            for tt in t:
                all_words.append(tt)

        pred_tag_lists = pred_tag_lists[train_word_count:]
        for i in range(0, len(pred_tag_lists) - 1):
            if pred_tag_lists[i] == "O":
                if pred_tag_lists[i + 1].startswith("M-") or pred_tag_lists[i + 1].startswith("E-"):
                    pos: str = pred_tag_lists[i + 1][2:]
                    pred_tag_lists[i + 1] = "B-" + pos
            elif pred_tag_lists[i].startswith("B-") or pred_tag_lists[i].startswith("M-"):
                pos: str = pred_tag_lists[i][2:]
                if pred_tag_lists[i + 1].startswith("M-"):
                    pred_tag_lists[i + 1] = "M-" + pos
                else:
                    pred_tag_lists[i + 1] = "E-" + pos
            elif pred_tag_lists[i].startswith("E-"):
                pos: str = pred_tag_lists[i][2:]
                if pred_tag_lists[i + 1].startswith("M-"):
                    pred_tag_lists[i + 1] = "B-" + pos
                elif pred_tag_lists[i + 1].startswith("E-"):
                    pred_tag_lists[i + 1] = "O"

        return pred_tag_lists


if __name__ == '__main__':
    train_word_lists, train_tag_lists, train_word2id, train_tag2id = data_build(file_name="ner_char_data/train.txt",
                                                                                make_vocab=True)
    test_word_lists, test_tag_lists, test_word2id, test_tag2id = data_build(file_name="ner_char_data/test.txt",
                                                                            make_vocab=True)

    # 训练模型
    me_model = MEModel()
    me_model.train(train_word_lists, train_tag_lists, test_word_lists, test_tag_lists, test_word2id)

    pred_tag_lists: list[str] = me_model.test(train_word_lists, test_word_lists)

    with open("result-ner-char.txt", "w") as f:
        with open("ner_char_data/test.txt", "r") as tf:
            count = 0
            for line in tf:
                if line == "\n":
                    f.write("\n")
                else:
                    f.write("字 " + pred_tag_lists[count] + "\n")
                    count += 1
