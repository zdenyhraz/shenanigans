close all;
clearvars;
clc;

M=readmatrix('D:\MainOutput\S-wind\features\features.csv');%header is omitted automatically by readmatrix

X=M(:,1);
Y=M(:,2);
SPD=M(:,3);
DIR=M(:,4);
U=SPD.*cos(deg2rad(DIR));
V=-SPD.*sin(deg2rad(DIR));%reversed

width=300;
height=200;
arrows=20;
contours=20;
contourwidth=1.0;
arrowscale=1.5;
arrowwidth=1.0;
arrowcolor='white';

[XMESH,YMESH] = meshgrid(0:width-1,0:height-1);
[QXMESH,QYMESH] = meshgrid(linspace(0,width-1,arrows),linspace(0,height-1,arrows));


%=============================== PLOTS ===============================
figwidth=800;
figheight=600;
offset=50;
hspace=figwidth+20;
vspace=figheight+100;

%points
figure('Position',[offset offset figwidth figheight])
scatter(X,Y,'filled');
hold on
quiver(X,Y,U,V,arrowscale,'LineWidth',arrowwidth,'Color',[0.4660 0.6740 0.1880])
hold off
xlim([0,width-1]);
ylim([0,height-1]);
axis ij
colormap jet
title('PTS')

%speeds
figure('Position',[offset offset+vspace figwidth figheight])
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
contourf(XMESH,YMESH,griddata(X,Y,SPD,XMESH,YMESH,'v4'),contours,'LineWidth',contourwidth);
hold on
quiver(QXMESH,QYMESH,griddata(X,Y,U,QXMESH,QYMESH,'v4'),griddata(X,Y,V,QXMESH,QYMESH,'v4'),arrowscale,'LineWidth',arrowwidth,'Color',arrowcolor)
hold off
axis ij
title('v4')
colormap jet
xlim([0,width-1]);
ylim([0,height-1]);
colorbar




