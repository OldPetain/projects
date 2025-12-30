# draw_metrics.py 用于绘制 ROC 曲线图和评估指标的主函数
import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt
from sklearn.metrics import (
    roc_curve, auc,
    accuracy_score, precision_score, recall_score, f1_score
)
from sklearn.preprocessing import label_binarize

# 模型名称列表
models = ['knn','decision_tree','mlp','svm','cnn','cnn_svm']
num_classes = 10

def compute_metrics(y_true, y_pred, y_prob):
    acc = accuracy_score(y_true, y_pred)
    prec = precision_score(y_true, y_pred, average='macro', zero_division=0)
    rec = recall_score(y_true, y_pred, average='macro', zero_division=0)
    f1 = f1_score(y_true, y_pred, average='macro', zero_division=0)

    y_true_bin = label_binarize(y_true, classes=range(num_classes))
    fpr, tpr, roc_auc = {}, {}, {}

    for i in range(num_classes):
        fpr[i], tpr[i], _ = roc_curve(y_true_bin[:, i], y_prob[:, i])
        roc_auc[i] = auc(fpr[i], tpr[i])

    all_fpr = np.unique(np.concatenate([fpr[i] for i in range(num_classes)]))
    mean_tpr = np.zeros_like(all_fpr)
    for i in range(num_classes):
        mean_tpr += np.interp(all_fpr, fpr[i], tpr[i])
    mean_tpr /= num_classes
    macro_auc = auc(all_fpr, mean_tpr)

    return acc, prec, rec, f1, macro_auc, (all_fpr, mean_tpr)

def draw_roc_curves(metrics_dict):
    plt.figure(figsize=(8, 6))
    for model, vals in metrics_dict.items():
        fpr, tpr = vals['roc']
        auc_score = vals['auc']
        plt.plot(fpr, tpr, label=f"{model.upper()} (AUC = {auc_score:.3f})")

    plt.plot([0, 1], [0, 1], 'k--')
    plt.xlabel("False Positive Rate")
    plt.ylabel("True Positive Rate")
    plt.title("ROC Curve Comparison")
    plt.legend()
    plt.grid()
    plt.tight_layout()
    plt.savefig("output/roc_comparison.png")
    print("ROC curve saved to output/roc_comparison.png")

def main():
    os.makedirs("output", exist_ok=True)
    summary = []
    metrics_for_plot = {}

    for model in models:
        path = f"output/{model}_prob_output.csv"
        if not os.path.exists(path):
            print(f"[WARN] File not found: {path}")
            summary.append({
                "Model": model,
                "Accuracy": "------",
                "Precision (Macro)": "------",
                "Recall (Macro)": "------",
                "F1 (Macro)": "------",
                "AUC (Macro)": "------"
            })
            continue

        df = pd.read_csv(path)
        y_true = df['label'].values
        y_prob = df.drop(columns=['label']).values
        y_pred = np.argmax(y_prob, axis=1)

        acc, prec, rec, f1, macro_auc, roc_data = compute_metrics(y_true, y_pred, y_prob)

        summary.append({
            "Model": model,
            "Accuracy": round(acc, 4),
            "Precision (Macro)": round(prec, 4),
            "Recall (Macro)": round(rec, 4),
            "F1 (Macro)": round(f1, 4),
            "AUC (Macro)": round(macro_auc, 4)
        })

        metrics_for_plot[model] = {'roc': roc_data, 'auc': macro_auc}

    # 保存评估指标 CSV
    df_summary = pd.DataFrame(summary)
    df_summary.to_csv("output/metrics_summary.csv", index=False)
    print("Metrics summary saved to output/metrics_summary.csv")

    # 绘制 ROC 曲线图
    if metrics_for_plot:
        draw_roc_curves(metrics_for_plot)
    else:
        print("No valid ROC data available to plot.")

if __name__ == "__main__":
    main()