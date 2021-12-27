import json

if __name__ == '__main__':
    clue_files = ["ner_clue_data/train.txt", "ner_clue_data/dev.txt"]
    char_files = ["ner_clue_data/train_char.txt", "ner_clue_data/dev_char.txt"]
    for j in range(0, len(clue_files)):
        with open(char_files[j], "w") as wf:
            with open(clue_files[j], "r") as f:
                for line_str in f:
                    line: dict = json.loads(line_str)
                    text: str = line["text"]
                    tag_list: list[str] = ["O"] * len(text)
                    label: dict = line["label"]
                    for label_item in label.keys():
                        for item in label[label_item]:
                            for lst in label[label_item][item]:
                                for i in range(lst[0], lst[1] + 1):
                                    if i == lst[0]:
                                        tag_list[i] = "B-" + label_item
                                    elif i == lst[1]:
                                        tag_list[i] = "E-" + label_item
                                    else:
                                        tag_list[i] = "M-" + label_item

                    for i in range(0, len(text)):
                        wf.write(text[i] + " " + tag_list[i] + "\n")
                    wf.write("\n")
