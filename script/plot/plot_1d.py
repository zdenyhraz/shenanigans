import matplotlib.pyplot as plt
fig, ax1 = plt.subplots(num=id)
if fig.get_size_inches()[1] < 6:
  fig.set_size_inches(fig.get_size_inches()*1.5)

# fig.clf()
ax1.clear()
ax2 = ax1.twinx() if (y2 or y2s) else None
if ax2:
  ax2.clear()
lines = []

if y:
  lines += ax1.plot(x, y, linestyle_y, label=label_y if label_y else "")
if y2:
  lines += ax2.plot(x, y2, linestyle_y2, label=label_y2 if label_y2 else "")
if ys:
  for i in range(0, len(ys)):
    lines += ax1.plot(x, ys[i], linestyle_ys[i] if len(linestyle_ys) >
                      i else "", label=label_ys[i] if len(label_ys) > i else "")
if y2s:
  for i in range(0, len(y2s)):
    lines += ax2.plot(x, y2s[i], linestyle_y2s[i] if len(linestyle_y2s) >
                      i else "", label=label_y2s[i] if len(label_y2s) > i else "")

if xlabel:
  ax1.set_xlabel(xlabel)
if ylabel:
  ax1.set_ylabel(ylabel)
if ax2 and y2label:
  ax2.set_ylabel(y2label)
if title:
  plt.title(title)
if label_y or label_ys or label_y2 or label_y2s:
  labels = [line.get_label() for line in lines]
  plt.legend(lines, labels)

plt.draw()
plt.pause(1e-9)
