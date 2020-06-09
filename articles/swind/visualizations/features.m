close all;
clearvars;
clc;

M=readmatrix('E:\Zdeny_PhD_Shenanigans\articles\swind\csv\features.csv');%header is omitted automatically by readmatrix
IMG=readmatrix('E:\Zdeny_PhD_Shenanigans\articles\swind\csv\image.csv');

X=M(:,1);
Y=M(:,2);
SPD=M(:,3);
DIR=M(:,4);
U=SPD.*cos(deg2rad(DIR));
V=-SPD.*sin(deg2rad(DIR));%reversed

width=300;
height=200;
arrows=20;
contours=10;
contourwidth=1.0;
arrowscale=1.5;
arrowwidth=1.0;
arrowcolor='magenta';

[XMESH,YMESH] = meshgrid(0:width-1,0:height-1);
[QXMESH,QYMESH] = meshgrid(linspace(0,width-1,arrows),linspace(0,height-1,arrows));


%=============================== PLOTS ===============================
figwidth=800;
figheight=600;
offset=50;
hspace=figwidth+20;
vspace=figheight+100;

%image
figure('Position',[offset offset+vspace figwidth figheight])
contourf(XMESH,YMESH,IMG,256,'LineStyle','none');
hold on
quiver(X,Y,U,V,arrowscale,'LineWidth',arrowwidth,'Color','red')
hold off
xlim([0,width-1]);
ylim([0,height-1]);
axis ij
colormap gray
title('image + arrows')

%== speeds==

figure('Position',[offset+hspace offset figwidth figheight])
contourf(XMESH,YMESH,griddata(X,Y,SPD,XMESH,YMESH,'linear'),contours,'LineWidth',contourwidth);
hold on
quiver(QXMESH,QYMESH,griddata(X,Y,U,QXMESH,QYMESH,'linear'),griddata(X,Y,V,QXMESH,QYMESH,'linear'),arrowscale,'LineWidth',arrowwidth,'Color',arrowcolor)
hold off
axis ij
title('linear')
colormap jet
xlim([0,width-1]);
ylim([0,height-1]);
colorbar

figure('Position',[offset+hspace offset+vspace figwidth figheight])
contourf(XMESH,YMESH,griddata(X,Y,SPD,XMESH,YMESH,'natural'),contours,'LineWidth',contourwidth);
hold on
quiver(QXMESH,QYMESH,griddata(X,Y,U,QXMESH,QYMESH,'natural'),griddata(X,Y,V,QXMESH,QYMESH,'natural'),arrowscale,'LineWidth',arrowwidth,'Color',arrowcolor)
hold off
axis ij
title('natural')
colormap jet
xlim([0,width-1]);
ylim([0,height-1]);
colorbar

figure('Position',[offset+2*hspace offset figwidth figheight])
contourf(XMESH,YMESH,griddata(X,Y,SPD,XMESH,YMESH,'cubic'),contours,'LineWidth',contourwidth);
hold on
quiver(QXMESH,QYMESH,griddata(X,Y,U,QXMESH,QYMESH,'cubic'),griddata(X,Y,V,QXMESH,QYMESH,'cubic'),arrowscale,'LineWidth',arrowwidth,'Color',arrowcolor)
hold off
axis ij
title('cubic')
colormap jet
xlim([0,width-1]);
ylim([0,height-1]);
colorbar

figure('Position',[offset+2*hspace offset+vspace figwidth figheight])
contourf(XMESH,YMESH,griddata(X,Y,SPD,XMESH,YMESH,'nearest'),contours,'LineWidth',contourwidth);
hold on
quiver(QXMESH,QYMESH,griddata(X,Y,U,QXMESH,QYMESH,'nearest'),griddata(X,Y,V,QXMESH,QYMESH,'nearest'),arrowscale,'LineWidth',arrowwidth,'Color',arrowcolor)
hold off
axis ij
title('nearest')
colormap jet
xlim([0,width-1]);
ylim([0,height-1]);
colorbar

%== all in one ==

figure('Position',[offset offset figwidth figheight])
title('article')
ax1 = axes;
image(ax1,IMG,'CDataMapping','direct');
ax2 = axes;
axis ij
hold on
contour(ax2,XMESH,YMESH,griddata(X,Y,SPD,XMESH,YMESH,'natural'),contours,'LineWidth',contourwidth);
quiver(ax2,QXMESH,QYMESH,griddata(X,Y,U,QXMESH,QYMESH,'natural'),griddata(X,Y,V,QXMESH,QYMESH,'natural'),arrowscale,'LineWidth',arrowwidth,'Color','magenta')
hold off
linkaxes([ax1,ax2])
ax2.Visible = 'off';
ax2.XTick = [];
ax2.YTick = [];
colormap(ax1,'gray')
colormap(ax2,'jet')
set([ax1,ax2],'Position',[.08 .11 .775 .815]);
cb2 = colorbar(ax2,'Position',[.88 .11 .0375 .815]);
xlim([0,width-1]);
ylim([0,height-1]);






