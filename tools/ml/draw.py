import numpy as np
import cv2


def draw_prediction(frame, rect, label, score, color):
    rows = frame.shape[0]
    boxThickness = int(np.clip(0.005 * rows, 1., 100.))
    fontThickness = int(np.clip(0.002 * rows, 1., 100.))
    fontScale = 1.5 * rows / 1600
    font = cv2.FONT_HERSHEY_SIMPLEX
    labelPaddingX = 0.02
    labelPaddingY = 0.5
    tl = (int(rect[0].item()), int(rect[1].item()))
    br = (int(rect[2].item()), int(rect[3].item()))

    cv2.rectangle(frame, tl, br, color, boxThickness)

    labelText = f"{label}: {int(score * 100)}%"
    labelSize, baseLine = cv2.getTextSize(labelText, font, fontScale, fontThickness)
    cv2.rectangle(frame, (int(tl[0] - 0.5 * boxThickness), int(tl[1] - labelSize[1] * (1. + labelPaddingY))),
                  (int(tl[0] + labelSize[0] * (1. + labelPaddingX)), int(tl[1])), color, cv2.FILLED)
    cv2.putText(frame, labelText, (int(tl[0] + 0.5 * labelPaddingX * labelSize[0]), int(tl[1] - 0.5 *
                labelPaddingY * labelSize[1])), font, fontScale, (0, 0, 0), fontThickness, cv2.LINE_AA)
