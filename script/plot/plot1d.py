import matplotlib.pyplot as plt

fig = plt.figure(num=id)
fig.set_figwidth(fig.get_figwidth()*aspectratio)
fig.canvas.manager.set_window_title(title)
plt.clf()

ax1 = fig.subplots()
ax2 = ax1.twinx() if (y2 or y2s) else None
color_cycle = ax1._get_lines.prop_cycler
lines = []

if y:
  lines += ax1.plot(x, y, color=color_y if color_y else next(color_cycle)['color'],
                    linestyle=linestyle_y, label=label_y, linewidth=4)
if y2:
  lines += ax2.plot(x, y2, color=color_y2 if color_y2 else next(color_cycle)['color'],
                    linestyle=linestyle_y2, label=label_y2, linewidth=4)
if ys:
  for i in range(0, len(ys)):
    if len(ys[i]) > 0:
      lines += ax1.plot(x, ys[i],
                        color=color_ys[i] if len(color_ys) > i else next(color_cycle)['color'], linestyle=linestyle_ys
                        [i] if len(linestyle_ys) > i else '-', label=label_ys[i] if len(label_ys) > i else '', linewidth=4)
if y2s:
  for i in range(0, len(y2s)):
    if len(y2s[i]) > 0:
      lines += ax2.plot(x, y2s[i],
                        color=color_y2s[i] if len(color_y2s) > i else next(color_cycle)['color'],
                        linestyle=linestyle_y2s[i] if len(linestyle_y2s) > i else '-', label=label_y2s[i]
                        if len(label_y2s) > i else '', linewidth=4)

if xlabel:
  ax1.set_xlabel(xlabel)
if ylabel:
  ax1.set_ylabel(ylabel)
if ax2 and y2label:
  ax2.set_ylabel(y2label)
if label_y or label_ys or label_y2 or label_y2s:
  labels = [line.get_label() for line in lines]
  plt.legend(lines, labels)
if log:
  plt.yscale("log")

plt.title(title)
plt.tight_layout()

if save:
  plt.savefig(save, bbox_inches='tight')
else
  plt.draw()
  plt.pause(1e-9)

