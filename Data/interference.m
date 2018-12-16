x=0:0.01:5;
y1=sin(10*x);
y2=sin(10*x+2.28169);
figure;
plot(x,y1);
hold on
plot(x,y2);
hold on
plot(x,y1+y2);
legend('y1','y2','y1+y2');
