// https://www.saiensu.co.jp/book_support/978-4-86481-061-6/psu.html
// https://www1.doshisha.ac.jp/~jmizushi/ps.html
// https://www1.doshisha.ac.jp/~jmizushi/ps/NewPostScriptManual.pdf

double xsize, ysize;		// papersize
double xv1, yv1, xv2, yv2;	// viewpoint
double xw1, yw1, xw2, yw2;	// world coordinate

FILE *stream;

void PS_setgray(double g)
{
	fprintf(stream, "%lf setgray\n", g);
	return;
}

void PS_setrgb(double r, double g, double b)
{
	fprintf(stream, "%lf %lf %lf setrgbcolor\n", r, g, b);
	return;
}

void PS_setcmyk(double c, double m, double y, double k)
{
	fprintf(stream, "%lf %lf %lf %lf setcmykcolor\n", c, m, y, k);
	return;
}

void PS_stroke()
{
	fprintf(stream, "stroke\n");
}

void PS_newpath()
{
	fprintf(stream, "newpath\n");
	return;
}

void PS_closepath()
{
	fprintf(stream, "closepath\n");
	return;
}

void PS_fill()
{
	fprintf(stream, "fill\n");
	return;
}

void PS_plot(double x, double y, int ipen)
{
	double x1, y1;

	x1 = ((x-xw1)/(xw2-xw1)*(xv2-xv1) + xv1)*xsize;
	y1 = ((y-yw1)/(yw2-yw1)*(yv2-yv1) + yv1)*ysize;
	switch (ipen) {
	case 3:
		fprintf(stream, "%lf cm %lf cm moveto\n", x1, y1);
		break;
	case 2:
		fprintf(stream, "%lf cm %lf cm lineto\n", x1, y1);
		break;
	}
	return;
}

void PS_plotrot(double x, double y, double t, int ipen)
{
	double x1, y1, t1;

	t1 = t*3.14159/180.0;
	x1 = x*cos(t1)-y*sin(t1);
	y1 = x*sin(t1)+y*cos(t1);
	x1 = ((x1-xw1)/(xw2-xw1)*(xv2-xv1) + xv1)*xsize;
	y1 = ((y1-yw1)/(yw2-yw1)*(yv2-yv1) + yv1)*ysize;
	switch (ipen) {
	case 3:
		fprintf(stream, "%lf cm %lf cm moveto\n", x1,y1);
		break;
	case 2:
		fprintf(stream, "%lf cm %lf cm lineto\n", x1,y1);
		break;
	}
	return;
}

void PS_spline(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, int ipart)
{
	int ns=10, i;
	double u, x, y;

	PS_newpath();
	u=(double)ipart;
	x=-u*(u-1.)*(u-2.)/6.*x1+(u+1.)*(u-1.)*(u-2.)/2.*x2-(u+1.)*u*(u-2.)/2.*x3+(u+1.)*u*(u-1.)/6.*x4;
	y=-u*(u-1.)*(u-2.)/6.*y1+(u+1.)*(u-1.)*(u-2.)/2.*y2-(u+1.)*u*(u-2.)/2.*y3+(u+1.)*u*(u-1.)/6.*y4;
	PS_plot(x, y, 3);
	for (i=1; i<=ns; i++) {
		u=(double)ipart +(double)i/(double)ns;
		x=-u*(u-1.)*(u-2.)/6.*x1+(u+1.)*(u-1.)*(u-2.)/2.*x2-(u+1.)*u*(u-2.)/2.*x3+(u+1.)*u*(u-1.)/6.*x4;
		y=-u*(u-1.)*(u-2.)/6.*y1+(u+1.)*(u-1.)*(u-2.)/2.*y2-(u+1.)*u*(u-2.)/2.*y3+(u+1.)*u*(u-1.)/6.*y4;
		PS_plot(x, y, 2);
	}
	PS_stroke();
	return;
}

void PS_rect(double x1, double y1, double x2, double y2)
{
	PS_newpath();
	PS_plot(x1, y1, 3);
	PS_plot(x2, y1, 2);
	PS_plot(x2, y2, 2);
	PS_plot(x1, y2, 2);
	PS_closepath();
	return;
}

void PS_rectrot(double x1, double y1, double x2, double y2, double t)
{
	PS_newpath();
	PS_plotrot(x1, y1, t, 3);
	PS_plotrot(x2, y1, t, 2);
	PS_plotrot(x2, y2, t, 2);
	PS_plotrot(x1, y2, t, 2);
	PS_closepath();
	return;
}

/*void PS_rectround(double x1, double y1, double x2, double y2, double r1)
{
	PS_newpath();
	PS_plot((x1+x2)/2.0, y1, 3);
	arcto(x2, y1, x2, y2, r1);
	arcto(x2, y2, x1, y2, r1);
	arcto(x1, y2, x1, y1, r1);
	arcto(x1, y1, x2, y1, r1);
	PS_closepath();
	return;
}

void PS_rectroundrot(double x1, double y1, double x2, double y2, double r1, double t1)
{
	PS_newpath();
	PS_plotrot((x1+x2)/2.0, y1, t1, 3);
	arctorot(x2, y1, x2, y2, r1, t1);
	arctorot(x2, y2, x1, y2, r1, t1);
	arctorot(x1, y2, x1, y1, r1, t1);
	arctorot(x1, y1, x2, y1, r1, t1);
	PS_closepath();
	return;
}*/

void PS_circ(double x1, double y1, double r1)
{
	double xx1, yy1, rr1;
	xx1 = ((x1-xw1)/(xw2-xw1)*(xv2-xv1) + xv1)*xsize;
	yy1 = ((y1-yw1)/(yw2-yw1)*(yv2-yv1) + yv1)*ysize;
	rr1 = r1/(xw2-xw1)*(xv2-xv1)*xsize;
	PS_newpath();
	fprintf(stream, "%lf cm %lf cm %lf cm 0 360 arc\n", xx1, yy1, rr1);
	return;
}

void PS_circn(double x1, double y1, double r1)
{
	double xx1, yy1, rr1;
	xx1 = ((x1-xw1)/(xw2-xw1)*(xv2-xv1) + xv1)*xsize;
	yy1 = ((y1-yw1)/(yw2-yw1)*(yv2-yv1) + yv1)*ysize;
	rr1 = r1/(xw2-xw1)*(xv2-xv1)*xsize;
	PS_newpath();
	fprintf(stream, "%lf cm %lf cm %lf cm 0 360 arcn\n", xx1, yy1, rr1);
	return;
}

void PS_line(double x, double y, double xx, double yy)
{
	PS_plot(x,  y,  3);
	PS_plot(xx, yy, 2);
	return;
}

void PS_linerot(double x, double y, double xx, double yy, double t)
{
	PS_plotrot(x,  y,  t, 3);
	PS_plotrot(xx, yy, t, 2);
	return;
}

void PS_linety(int ichar)
{
	switch (ichar) {
	case 1:
		fprintf(stream, "[] 0 setdash \n");
		break;
	case 2:
		fprintf(stream, "[2 2] 0 setdash \n");
		break;
	case 3:
		fprintf(stream, "[4 2] 0 setdash \n");
		break;
	case 4:
		fprintf(stream, "[8 2] 0 setdash \n");
		break;
	case 5:
		fprintf(stream, "[16 4] 0 setdash \n");
		break;
	case 6:
		fprintf(stream, "[32 8] 0 setdash \n");
		break;
	case 7:
		fprintf(stream, "[4 1 1 1] 0 setdash \n");
		break;
	case 8:
		fprintf(stream, "[8 1 2 1] 0 setdash \n");
		break;
	case 9:
		fprintf(stream, "[16 1 4 1] 0 setdash \n");
		break;
	}
	return;
}

void PS_linewidth(double w)
{
	fprintf(stream, "%lf setlinewidth\n", w);
	return;
}

void PS_arrow(double x1, double y1, double x2, double y2, double d)
{
	double alpha=2.2,s,x3,y3,x4,y4;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x3=x2-(x2-x1)/s*d*alpha-(y2-y1)/s*d;
	y3=y2-(y2-y1)/s*d*alpha+(x2-x1)/s*d;
	x4=x2-(x2-x1)/s*d*alpha+(y2-y1)/s*d;
	y4=y2-(y2-y1)/s*d*alpha-(x2-x1)/s*d;
	PS_plot(x1, y1, 3);
	PS_plot(x2, y2, 2);
	PS_plot(x3, y3, 3);
	PS_plot(x2, y2, 2);
	PS_plot(x4, y4, 2);
	return;
}

void PS_arrowa(double x1, double y1, double x2, double y2, double d)
{
	double alpha=2.2,s,x3,y3,x4,y4;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x3=x2-(x2-x1)/s*d*alpha-(y2-y1)/s*d;
	y3=y2-(y2-y1)/s*d*alpha+(x2-x1)/s*d;
	x4=x2-(x2-x1)/s*d*alpha+(y2-y1)/s*d;
	y4=y2-(y2-y1)/s*d*alpha-(x2-x1)/s*d;
	PS_plot(x1, y1, 3);
	PS_plot((x3+x4)/2.0, (y3+y4)/2.0, 2);
	PS_stroke();
	PS_plot(x3, y3, 3);
	PS_plot(x2, y2, 2);
	PS_plot(x4, y4, 2);
	PS_closepath();
	PS_fill();
	return;
}

void PS_arrowb(double x1, double y1, double x2, double y2, double d)
{
	double alpha=2.2,s,x3,y3,x4,y4,x32,y32,x24,y24,x43,y43;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x3=x2-(x2-x1)/s*d*alpha-(y2-y1)/s*d;
	y3=y2-(y2-y1)/s*d*alpha+(x2-x1)/s*d;
	x4=x2-(x2-x1)/s*d*alpha+(y2-y1)/s*d;
	y4=y2-(y2-y1)/s*d*alpha-(x2-x1)/s*d;
	x32=(x3+x2)/2.0-(y3-y2)/12.0;
	y32=(y3+y2)/2.0+(x3-x2)/12.0;
	x24=(x2+x4)/2.0-(y2-y4)/12.0;
	y24=(y2+y4)/2.0+(x2-x4)/12.0;
	x43=(x4+x3)/2.0-(y4-y3)/12.0;
	y43=(y4+y3)/2.0+(x4-x3)/12.0;
	PS_plot(x1, y1, 3);
	PS_plot(x43, y43, 2);
	PS_stroke();
	PS_plot(x3, y3, 3);
	PS_plot(x32, y32, 2);
	PS_plot(x2, y2, 2);
	PS_plot(x24, y24, 2);
	PS_plot(x4, y4, 2);
	PS_plot(x43,y43,2);
	PS_closepath();
	PS_fill();
	return;
}

void PS_arrowc(double x1, double y1, double x2, double y2, double d)
{
	double alpha=2.2,s,x3,y3,x4,y4,x5,y5;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x3=x2-(x2-x1)/s*d*alpha-(y2-y1)/s*d;
	y3=y2-(y2-y1)/s*d*alpha+(x2-x1)/s*d;
	x4=x2-(x2-x1)/s*d*alpha+(y2-y1)/s*d;
	y4=y2-(y2-y1)/s*d*alpha-(x2-x1)/s*d;
	x5=x2-(x2-x1)/s*d*alpha*0.8;
	y5=y2-(y2-y1)/s*d*alpha*0.8;
	PS_plot(x1, y1, 3);
	PS_plot(x5, y5, 2);
	PS_stroke();
	PS_plot(x3, y3, 3);
	PS_plot(x2, y2, 2);
	PS_plot(x4, y4, 2);
	PS_plot(x5, y5, 2);
	PS_closepath();
	PS_fill();
	return;
}

void PS_arrowPS_fill(double x1, double y1, double x2, double y2, double d)
{
	double alpha=2.2,s,x3,y3,x4,y4,x5,y5;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x3=x2-(x2-x1)/s*d*alpha-(y2-y1)/s*d;
	y3=y2-(y2-y1)/s*d*alpha+(x2-x1)/s*d;
	x4=x2-(x2-x1)/s*d*alpha+(y2-y1)/s*d;
	y4=y2-(y2-y1)/s*d*alpha-(x2-x1)/s*d;
	PS_plot(x1, y1, 3);
	PS_plot(x2, y2, 2);
	PS_stroke();
	PS_plot(x3, y3, 3);
	PS_plot(x2, y2, 2);
	PS_plot(x4, y4, 2);
	PS_closepath();
	PS_fill();
	return;
}

void PS_arrowrot(double x1, double y1, double x2, double y2, double t1, double d)
{
	double alpha=2.2,s,x3,y3,x4,y4;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x3=x2-(x2-x1)/s*d*alpha-(y2-y1)/s*d;
	y3=y2-(y2-y1)/s*d*alpha+(x2-x1)/s*d;
	x4=x2-(x2-x1)/s*d*alpha+(y2-y1)/s*d;
	y4=y2-(y2-y1)/s*d*alpha-(x2-x1)/s*d;
	PS_plotrot(x1, y1, t1, 3);
	PS_plotrot(x2, y2, t1, 2);
	PS_plotrot(x3, y3, t1, 3);
	PS_plotrot(x2, y2, t1, 2);
	PS_plotrot(x4, y4, t1, 2);
	return;
}

void PS_arrowwide(double x1, double y1, double x2, double y2, double d, double al)
{
	double s,dx,dy,x3,y3,x4,y4,x5,y5,x6,y6,x7,y7,x8,y8;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	dx=al;
	dy=al;
	x3=x2-(x2-x1)*d-(y2-y1)/s*dx;
	y3=y2-(y2-y1)*d+(x2-x1)/s*dy;
	x4=x2-(x2-x1)*d+(y2-y1)/s*dx;
	y4=y2-(y2-y1)*d-(x2-x1)/s*dy;
	x7=x2-(x2-x1)*d-(y2-y1)/s*dx*0.4;
	y7=y2-(y2-y1)*d+(x2-x1)/s*dy*0.4;
	x8=x2-(x2-x1)*d+(y2-y1)/s*dx*0.4;
	y8=y2-(y2-y1)*d-(x2-x1)/s*dy*0.4;
	x5=x1-(y2-y1)/s*dx*0.4;
	y5=y1+(x2-x1)/s*dy*0.4;
	x6=x1+(y2-y1)/s*dx*0.4;
	y6=y1-(x2-x1)/s*dy*0.4;
	PS_plot(x6, y6, 3);
	PS_plot(x8, y8, 2);
	PS_plot(x4, y4, 2);
	PS_plot(x2, y2, 2);
	PS_plot(x3, y3, 2);
	PS_plot(x7, y7, 2);
	PS_plot(x5, y5, 2);
	PS_closepath();
	PS_stroke();
	return;
}

void PS_text(double x, double y, char* str1)
{
	double x1, y1;

	x1 = ((x-xw1)/(xw2-xw1)*(xv2-xv1) + xv1)*xsize;
	y1 = ((y-yw1)/(yw2-yw1)*(yv2-yv1) + yv1)*ysize;
	fprintf(stream, "%lf cm %lf cm moveto\n", x1, y1);
	fprintf(stream, "(%s) show\n", str1);
	return;
}

void PS_text1(double x, double y, char s1)
{
	double x1, y1;

	x1 = ((x-xw1)/(xw2-xw1)*(xv2-xv1) + xv1)*xsize;
	y1 = ((y-yw1)/(yw2-yw1)*(yv2-yv1) + yv1)*ysize;
	fprintf(stream, "%lf cm %lf cm moveto\n", x1, y1);
	fprintf(stream, "(%c) show\n", s1);
	return;
}

void PS_textx(double x, double y, char* str1)
{
	int n;
	n = strlen(str1);
//	printf("%d length\n", n);
	PS_plot(x, y, 3);
	fprintf(stream, "%lf %lf rmoveto\n", -n/2.0*9.0, -3.0/2.0*15.0);
	fprintf(stream, "(%s) show\n", str1);
	return;
}

void PS_xaxis(double x1, double y1, double x2, double y2, int m, int n)
{
	double s, d, x, dx, dy;
	PS_setgray(0.0);
	s = (x2-x1) * (x2-x1)+(y2-y1) * (y2-y1);
	s = sqrt(s);
//	d = s/120.0;//60.0;
//	PS_arrow(x1-s/20.0, y1, x2+s/20.0*2.0, y2, d);
	PS_arrowa(x1-s/20.0, y1, x2+s/20.0*2.0, y2, 0.04);

	double a = (x2-x1)/2;
	double o = a+x1;
	for (int i=0; i<=m*n; i++) {
		x = 1.0/(double)(m*n)*(double)i;
		if (i/n*n==i) dy=0.02;
		else dy=0.01;
		PS_line(-x*a+o, -dy, -x*a+o, dy);
		PS_line(x*a+o, -dy, x*a+o, dy);
	}
	PS_stroke();
/*	PS_textx(x2+s/10.0*2.0, -s/30.0, "x");
	PS_textx((x1+x2)/2.0, (y1+y2)/2.0, "O");
	PS_textx(x2/2.0, y2, "pi/2");
	PS_textx(x2, y2, "pi");
	PS_textx(x1/2.0, y1, "-pi/2");
	PS_textx(x1, y1, "-pi");
	PS_stroke();*/
	return;
}

void PS_viewport(double xv1d, double yv1d, double xv2d, double yv2d)
{
	xv1 = xv1d;
	yv1 = yv1d;
	xv2 = xv2d;
	yv2 = yv2d;
	return;
}

void PS_xyworld(double xw1d, double yw1d, double xw2d, double yw2d)
{
	xw1 = xw1d;
	yw1 = yw1d;
	xw2 = xw2d;
	yw2 = yw2d;
	return;
}

void PS_init(char *name)
{
	stream = fopen(name, "w");

	// aspect ratio of the paper is 1.4143
	fprintf(stream, "%%! Created by Yuichiro Nakada \n");
//	fprintf(stream, "/Times-Roman findfont 18 scalefont setfont \n");
	fprintf(stream, "/cm {28.35 mul} def \n");

	xsize = 21.0;/* ysize = 29.7;*/ ysize = 21.0;
	PS_viewport(0.2, 0.2, 0.8, 0.8);
	PS_xyworld(0.0, 0.0, 1.0, 1.0);
	PS_linety(1);
	PS_linewidth(1);
	return;
}

void PS_fin()
{
	PS_stroke();
	fprintf(stream, "showpage\n");
	fclose(stream);
	return;
}


#if 0
void PS_annulus(double x1,double y1,double r1,double r2)
{
	PS_plot(x1+r2, y1, 3);
	arc(x1, y1, r2, 1.0, 360.0);
	PS_plot(x1+r1, y1, 2);
	arcn(x1, y1, r1, 360.0, 0.0);
	PS_plot(x1+r2, y1, 2);
	PS_closepath();
}

void PS_arc(double x1, double y1, double r1, double t1, double t2)
{
	double xx1,yy1,rr1;

	xx1=((x1-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy1=((y1-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	rr1=(r1/(xw2-xw1)*(xv2-xv1))*xsize;
	fprintf(stream,"%lf cm %lf cm %lf cm %lf %lf arc \n",xx1,yy1,rr1,t1,t2);
	return;
}

void PS_arcn(double x1, double y1, double r1, double t1, double t2)
{
	double xx1,yy1,rr1;

	xx1=((x1-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy1=((y1-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	rr1=(r1/(xw2-xw1)*(xv2-xv1))*xsize;
	fprintf(stream,"%lf cm %lf cm %lf cm %lf %lf arcn \n",xx1,yy1,rr1,t1,t2);
	return;
}

void PS_arcto(double x1, double y1, double x2, double y2, double r1)
{
	double xx1,yy1,xx2,yy2,rr1;

	xx1=((x1-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy1=((y1-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	xx2=((x2-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy2=((y2-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	rr1=(r1/(xw2-xw1)*(xv2-xv1))*xsize;
	fprintf(stream,"%lf cm %lf cm %lf cm %lf cm %lf cm arcto \n",xx1,yy1,xx2,yy2,rr1);
	return;
}

void PS_arctorot(double x1, double y1, double x2, double y2, double r1, double t1)
{
	double xx1,yy1,xx2,yy2,rr1,tt1;
	tt1 = t1*3.14159/180.0;
	xx1 = x1*cos(tt1)-y1*sin(tt1);
	yy1 = x1*sin(tt1)+y1*cos(tt1);
	xx2 = x2*cos(tt1)-y2*sin(tt1);
	yy2 = x2*sin(tt1)+y2*cos(tt1);
	xx1=((xx1-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy1=((yy1-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	xx2=((xx2-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy2=((yy2-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	rr1=(r1/(xw2-xw1)*(xv2-xv1))*xsize;
	fprintf(stream,"%lf cm %lf cm %lf cm %lf cm %lf cm arcto \n",xx1,yy1,xx2,yy2,rr1);
	return;
}

void PS_battery(double x1, double y1, double x2, double y2, double d1, double d2)
{
	double s,dx,dy,ex,ey,xx,yy;

	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	dx=x2-x1;
	dy=y2-y1;
	ex=-dy;
	ey=dx;
	xx=x1+d1*ex;
	yy=y1+d1*ey;
	PS_plot(xx, yy, 3);
	xx=x1-d1*ex;
	yy=y1-d1*ey;
	PS_plot(xx, yy, 2);
	xx=x2+d2*ex;
	yy=y2+d2*ey;
	PS_plot(xx, yy, 3);
	xx=x2-d2*ex;
	yy=y2-d2*ey;
	PS_plot(xx, yy, 2);
	return;
}

void PS_brokenlines(double *x1, double *y1, int n)
{
	int i;
	PS_newpath();
	PS_plot(x1[0],y1[0],3);
	for (i=1; i<n; i++) {
		PS_plot(x1[i], y1[i], 2);
	}
	return;
}

void PS_clipoff()
{
	fprintf(stream, "clip\n");
}

void PS_clipon()
{
	fprintf(stream, "clip\n");
}

void PS_cliponrec(double x1, double y1, double x2, double y2)
{
	PS_newpath();
	PS_plot(x1, y1, 3);
	PS_plot(x2, y1, 2);
	PS_plot(x2, y2,2);
	PS_plot(x1, y2, 2);
	PS_closepath();
	fprintf(stream, "clip\n");
}

void PS_coil(double x1, double y1, double x2, double y2, double d, int n)
{
	double s,ex,ey,ds,a,xx,yy,t;
	int i;

	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	ex=(x2-x1)/s;
	ey=(y2-y1)/s;
	a=2.0;
	ds=(s-2.0*d)/((double) n +0.5);
	PS_plot(x1, y1, 3);
	for (i=1; i<=(40*n+20); i++) {
		t=2.0*3.14159*(double) i/40.0;
		xx=x1+((d-d*cos(t))+t/2.0/3.14159*ds)*ex-a*d*sin(t)*ey;
		yy=y1+((d-d*cos(t))+t/2.0/3.14159*ds)*ey+a*d*sin(t)*ex;
		PS_plot(xx, yy, 2);
	}
	return;
}

void PS_curvto(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double xx1,yy1,xx2,yy2,xx3,yy3;

	xx1=((x1-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy1=((y1-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	xx2=((x2-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy2=((y2-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	xx3=((x3-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
	yy3=((y3-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
	fprintf(stream, "%lf cm %lf cm %lf cm %lf cm %lf cm %lf cm curveto\n",xx1,yy1,xx2,yy2,xx3,yy3);
	return;
}

void PS_curvton(double * x1, double * y1, int n)
{
	double xx1[100],yy1[100];
	int i;

	for (i=0; i<n; i++) {
		xx1[i]=((x1[i]-xw1)/(xw2-xw1)*(xv2-xv1)+xv1)*xsize;
		yy1[i]=((y1[i]-yw1)/(yw2-yw1)*(yv2-yv1)+yv1)*ysize;
		fprintf(stream, "%lf cm %lf cm ",xx1[i],yy1[i]);
	}
	fprintf(stream, " curveto\n");
	return;
}

void PS_curvtona(double * x1, double * y1, double t, int n)
{
	double x12[100],y12[100],x21[100],y21[100];
	int i;

	for (i=0; i<n-1; i++) {
		x12[i]=t*x1[i]+(1.0-t)*x1[i+1];
		x21[i]=(1.0-t)*x1[i]+t*x1[i+1];
		y12[i]=t*y1[i]+(1.0-t)*y1[i+1];
		y21[i]=(1.0-t)*y1[i]+t*y1[i+1];
	}
	PS_plot(x21[0],y21[0], 2);
	for (i=0; i<n-2; i++) {
		curvto(x21[i],y21[i],x1[i+1],y1[i+1],x12[i+1],y12[i+1]);
	}
	PS_plot(x1[n-1],y1[n-1],2);
	return;
}

void PS_dims(double x1, double y1, double x2, double y2, double d)
{
	double x3,y3,x4,y4,s,t;
	int i,n;
	n=10;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x3=(x1+x2)*0.5-d*(y2-y1)/s;
	y3=(y1+y2)*0.5+d*(x2-x1)/s;
	x4=x1;
	y4=y1;
	PS_newpath();
	PS_plot(x4,y4,3);
	for (i=1; i<=n; i++) {
		t=0.7*(double)i/(double)n;
		x4=x1*(t-1.0)*(t-2.0)*0.5-x3*t*(t-2.0)+x2*t*(t-1.0)*0.5;
		y4=y1*(t-1.0)*(t-2.0)*0.5-y3*t*(t-2.0)+y2*t*(t-1.0)*0.5;
		PS_plot(x4,y4,2);
	};
	PS_stroke();
	x4=x2;
	y4=y2;
	PS_newpath();
	PS_plot(x4,y4,3);
	for (i=1; i<=n; i++) {
		t=0.7*(double)i/(double)n;
		x4=x2*(t-1.0)*(t-2.0)*0.5-x3*t*(t-2.0)+x1*t*(t-1.0)*0.5;
		y4=y2*(t-1.0)*(t-2.0)*0.5-y3*t*(t-2.0)+y1*t*(t-1.0)*0.5;
		PS_plot(x4,y4,2);
	};
	PS_stroke();
	return;
}

void PS_dimsrot(double x1, double y1, double x2, double y2,double d,double t1)
{
	double x3,y3,x4,y4,s,t;
	int i,n;
	n=10;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x3=(x1+x2)*0.5-d*(y2-y1)/s;
	y3=(y1+y2)*0.5+d*(x2-x1)/s;
	x4=x1;
	y4=y1;
	PS_newpath();
	PS_plotrot(x4,y4,t1,3);
	for (i=1; i<=n; i++) {
		t=0.7*(double)i/(double)n;
		x4=x1*(t-1.0)*(t-2.0)*0.5-x3*t*(t-2.0)+x2*t*(t-1.0)*0.5;
		y4=y1*(t-1.0)*(t-2.0)*0.5-y3*t*(t-2.0)+y2*t*(t-1.0)*0.5;
		PS_plotrot(x4,y4,t1,2);
	};
	PS_stroke();
	x4=x2;
	y4=y2;
	PS_newpath();
	PS_plotrot(x4,y4,t1,3);
	for (i=1; i<=n; i++) {
		t=0.7*(double)i/(double)n;
		x4=x2*(t-1.0)*(t-2.0)*0.5-x3*t*(t-2.0)+x1*t*(t-1.0)*0.5;
		y4=y2*(t-1.0)*(t-2.0)*0.5-y3*t*(t-2.0)+y1*t*(t-1.0)*0.5;
		PS_plotrot(x4,y4,t1,2);
	};
	PS_stroke();
	return;
}

void PS_ellipse(double x1, double y1, double rx, double ry)
{
	double x, y, dt;
	int i;
	PS_newpath();
	x=x1+rx;
	y=y1;
	dt=3.14156*2.0/40.0;
	PS_plot(x,y,3);
	for (i=1; i<=40; i++) {
		x=x1+rx*cos(dt*i);
		y=y1+ry*sin(dt*i);
		PS_plot(x,y,2);
	}
	return;
}

void PS_ellipserot(double x1, double y1, double rx, double ry, double t)
{
	double x, y, dt;
	int i;
	PS_newpath();
	x=x1+rx;
	y=y1;
	dt=3.14156*2.0/40.0;
	PS_plotrot(x,y,t,3);
	for (i=1; i<=40; i++) {
		x=x1+rx*cos(dt*i);
		y=y1+ry*sin(dt*i);
		PS_plotrot(x,y,t,2);
	}
	return;
}

void PS_eoclipon()
{
	fprintf(stream, "eoclip\n");
}

void PS_eocliponrec(double x1, double y1, double x2, double y2)
{
	PS_newpath();
	PS_plot(x1, y1, 3);
	PS_plot(x2, y1, 2);
	PS_plot(x2, y2,2);
	PS_plot(x1, y2, 2);
	PS_closepath();
	fprintf(stream, "eoclip\n");
}

void PS_grestore()
{
	PS_stroke();
	fprintf(stream, "grestore\n");
	fclose(stream);
	return;
}

void PS_gsave()
{
	PS_stroke();
	fprintf(stream, "gsave\n");
	fclose(stream);
	return;
}

void PS_lgcurves(double * x1, double *y1, int n)
{
	int ns=10,i,j;
	double u,x,y;

	for (j=0; j<n-1; j++) {
		u=(double) j-1;
		x=-u*(u-1.)*(u-2.)/6.*x1[0]+(u+1.)*(u-1.)*(u-2.)/2.*x1[1]-(u+1.)*u*(u-2.)/2.*x1[2]
		  +(u+1.)*u*(u-1.)/6.*x1[3];
		y=-u*(u-1.)*(u-2.)/6.*y1[0]+(u+1.)*(u-1.)*(u-2.)/2.*y1[1]-(u+1.)*u*(u-2.)/2.*y1[2]
		  +(u+1.)*u*(u-1.)/6.*y1[3];
		PS_plot(x,y,3);
		for (i=1; i<=ns; i++) {
			u=(double) j-1 +(double) i/(double) ns;
			x=-u*(u-1.)*(u-2.)/6.*x1[0]+(u+1.)*(u-1.)*(u-2.)/2.*x1[1]-(u+1.)*u*(u-2.)/2.*x1[2]
			  +(u+1.)*u*(u-1.)/6.*x1[3];
			y=-u*(u-1.)*(u-2.)/6.*y1[0]+(u+1.)*(u-1.)*(u-2.)/2.*y1[1]-(u+1.)*u*(u-2.)/2.*y1[2]
			  +(u+1.)*u*(u-1.)/6.*y1[3];
			PS_plot(x,y,2);
		}
	}
	return;
}

void PS_linecap(int ip)
{
	fprintf(stream, "%d setlinecap\n", ip);
	return;
}

void PS_parabola(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double u,x,y;
	int i,ns=20;
	u=0.0;
	PS_newpath();
	x=(u-1.)*(u-2.)/2.*x1-u*(u-2.)*x2+u*(u-1.)/2.*x3;
	y=(u-1.)*(u-2.)/2.*y1-u*(u-2.)*y2+u*(u-1.)/2.*y3;
	PS_plot(x,y,3);
	for (i=1; i<=2*ns; i++) {
		u=(double) i/(double) ns;
		x=(u-1.)*(u-2.)/2.*x1-u*(u-2.)*x2+u*(u-1.)/2.*x3;
		y=(u-1.)*(u-2.)/2.*y1-u*(u-2.)*y2+u*(u-1.)/2.*y3;
		PS_plot(x,y,2);
	}
	PS_stroke();
	return;
}

void PS_polygon(double x1, double y1, double r, int n)
{
	double x, y, dt;
	int i;
	PS_newpath();
	x=x1;
	y=y1+r;
	dt=3.14156*2.0/(double)n;
	PS_plot(x,y,3);
	for (i=1; i<=n; i++) {
		x=x1+r*sin(dt*i);
		y=y1+r*cos(dt*i);
		PS_plot(x,y,2);
	}
	return;
}

void PS_polygonrot(double x1, double y1, double r, double t1, int n)
{
	double x, y, t, dt;
	int i;
	PS_newpath();
	t=3.14156*t1/180.0;
	x=x1+r*sin(t1);
	y=y1+r*cos(t1);
	dt=3.14156*2.0/(double)n;
	PS_plot(x,y,3);
	for (i=1; i<=n; i++) {
		x=x1+r*sin(dt*i+t);
		y=y1+r*cos(dt*i+t);
		PS_plot(x,y,2);
	}
	return;
}

void PS_resist(double x1, double y1, double x2, double y2, double d)
{
	double s, dx, dy, ex, ey, xx, yy;
	int i;
	s=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	dx=(x2-x1)/12.0;
	dy=(y2-y1)/12.0;
	ex=-d*dy;
	ey=d*dx;
	PS_plot(x1, y1, 3);
	xx=x1+dx+ex;
	yy=y1+dy+ey;
	PS_plot(xx, yy, 2);
	for (i=1; i<=5; i++) {
		xx=x1+(i*2+1)*dx+pow(-1,i)*ex;
		yy=y1+(i*2+1)*dy+pow(-1,i)*ey;
		PS_plot(xx, yy, 2);
	}
	PS_plot(x2, y2, 2);
	return;
}

void PS_rotate(double theta)
{
	translateo(theta);
	fprintf(stream, "%lf rotate\n",theta);
}

void PS_rplot(double dx, double dy, int ipen)
{
	double dx1, dy1;

	dx1 = ((dx-xw1)/(xw2-xw1)*(xv2-xv1) )*xsize;
	dy1 = ((dy-yw1)/(yw2-yw1)*(yv2-yv1) )*ysize;
	switch (ipen) {
	case 3:
		fprintf(stream, "%lf cm %lf cm rmoveto\n", dx1,dy1);
		break;
	case 2:
		fprintf(stream, "%lf cm %lf cm rlineto\n", dx1,dy1);
		break;
	}
	return;
}

void PS_rPS_plotrot(double dx, double dy, double t, int ipen)
{
	double dx1, dy1, t1;

	t1 = t*3.14159/180.0;
	dx1 = dx*cos(t1)-dy*sin(t1);
	dy1 = dx*sin(t1)+dy*cos(t1);
	dx1 = ((dx1-xw1)/(xw2-xw1)*(xv2-xv1) )*xsize;
	dy1 = ((dy1-yw1)/(yw2-yw1)*(yv2-yv1) )*ysize;
	switch (ipen) {
	case 3:
		fprintf(stream, "%lf cm %lf cm rmoveto\n", dx1,dy1);
		break;
	case 2:
		fprintf(stream, "%lf cm %lf cm rlineto\n", dx1,dy1);
		break;
	}
	return;
}

void PS_scale(double tx, double ty)
{
	fprintf(stream, "%lf %lf scale\n",tx, ty);
}

void PS_setchar(int ichar, int ip)
{
	switch (ichar) {
	case 1:
		fprintf(stream, "/Times-Roman findfont %d scalefont setfont \n",ip);
		break;
	case 2:
		fprintf(stream, "/Times-Bold findfont %d scalefont setfont \n",ip);
		break;
	case 3:
		fprintf(stream, "/Times-Italic findfont %d scalefont setfont \n",ip);
		break;
	case 4:
		fprintf(stream, "/Times-BoldItalic findfont %d scalefont setfont \n",ip);
		break;
	case 5:
		fprintf(stream, "/Helvetica findfont %d scalefont setfont \n",ip);
		break;
	case 6:
		fprintf(stream, "/Helvetica-Bold findfont %d scalefont setfont \n",ip);
		break;
	case 7:
		fprintf(stream, "/Helvetica-Oblique findfont %d scalefont setfont \n",ip);
		break;
	case 8:
		fprintf(stream, "/Helvetica-BoldOblique findfont %d scalefont setfont \n",ip);
		break;
	case 9:
		fprintf(stream, "/Courier findfont %d scalefont setfont \n",ip);
		break;
	case 10:
		fprintf(stream, "/Courier-Bold findfont %d scalefont setfont \n",ip);
		break;
	case 11:
		fprintf(stream, "/Courier-Oblique findfont %d scalefont setfont \n",ip);
		break;
	case 12:
		fprintf(stream, "/Courier-BoldOblique findfont %d scalefont setfont \n",ip);
		break;
	case 13:
		fprintf(stream, "/Symbol findfont %d scalefont setfont \n",ip);
		break;
	}
	return;
}

void PS_setlinejoin(int ljoin)
{
	fprintf(stream, "%d setlinejoin\n",ljoin);
	return;
}

void PS_square(double x, double y, double ell)
{
	PS_plot(x-ell/2.0,y-ell/2.0,3);
	PS_plot(x+ell/2.0,y-ell/2.0,2);
	PS_plot(x+ell/2.0,y+ell/2.0,2);
	PS_plot(x-ell/2.0,y+ell/2.0,2);
	PS_closepath();
	return;
}

void PS_squarerot(double x, double y, double ell, double t)
{
	PS_plotrot(x-ell/2.0,y-ell/2.0,t,3);
	PS_plotrot(x+ell/2.0,y-ell/2.0,t,2);
	PS_plotrot(x+ell/2.0,y+ell/2.0,t,2);
	PS_plotrot(x-ell/2.0,y+ell/2.0,t,2);
	PS_closepath();
	return;
}

void PS_star(double x0, double y0, double r, double t, int n)
{
	double pi,dth,dthi,alpha;
	int i;
	if (n<3) {
		printf("can't make a polygon");
		return;
	}
	if (n==6) {
		alpha=0.575;
	} else if (n<=4) {
		alpha=0.24;
	} else {
		alpha=0.36;
	}
	pi=3.14159;
	dth=pi/(double)n;
	PS_newpath();
	PS_plot(x0+r*cos(t+pi/2.0),y0+r*sin(t+pi/2.0),3);
	for (i=1; i<=2*n-1 ; i++) {
		if (i%2==0) {
			dthi=dth*(double)i;
			PS_plot(x0+r*cos(t+pi/2.0+dthi),y0+r*sin(t+pi/2.0+dthi),2);
		} else {
			dthi=dth*(double)i;
			PS_plot(x0+alpha*r*cos(t+pi/2.0+dthi),y0+alpha*r*sin(t+pi/2.0+dthi),2);
		}
	}
	PS_closepath();
	return;
}

void PS_starx(double x0, double y0, double r, double t, int n)
{
	double pi,dth,dthi,alpha;
	int i;
	if (n<3) {
		printf("can't make a polygon");
		return;
	}
	pi=3.14159;
	dth=pi/(double)n*2.0;
	PS_newpath();
	PS_plot(x0+r*cos(t+pi/2.0),y0+r*sin(t+pi/2.0),3);
	if (n%2==1) {
		for (i=2 ; i<=2*n ; i=i+2) {
			dthi=dth*(double)i;
			PS_plot(x0+r*cos(t+pi/2.0+dthi),y0+r*sin(t+pi/2.0+dthi),2);
		}
		PS_closepath();
	} else {
		for (i=2 ; i<=n ; i=i+2) {
			dthi=dth*(double)i;
			PS_plot(x0+r*cos(t+pi/2.0+dthi),y0+r*sin(t+pi/2.0+dthi),2);
		}
		PS_closepath();
		i=1;
		dthi=dth*(double)i;
		PS_plot(x0+r*cos(t+pi/2.0+dthi),y0+r*sin(t+pi/2.0+dthi),3);
		for (i=3 ; i<=n-1 ; i=i+2) {
			dthi=dth*(double)i;
			PS_plot(x0+r*cos(t+pi/2.0+dthi),y0+r*sin(t+pi/2.0+dthi),2);
		}
		PS_closepath();
	}
	return;
}


void PS_textrot(double x, double y, double t, char* str1)
{
	rotate(t);
	text(x, y,str1);
	rotate(-t);
	return;
}

void PS_texty(double x, double y, char* str1)
{
	int n;
	n=strlen(str1);
	PS_plot(x, y, 3);
	fprintf(stream, "%lf %lf rmoveto\n", -(n+1)*9.0,-1.0/2.0*12.0);
	fprintf(stream, "(%s) show\n", str1);
	return;
}

void PS_translate(double x, double y)
{
	double x1,y1;
	x1 = ((x-xw1)/(xw2-xw1)*(xv2-xv1) + xv1)*xsize;
	y1 = ((y-yw1)/(yw2-yw1)*(yv2-yv1) + yv1)*ysize;
	fprintf(stream, "%lf cm %lf cm translate\n", x1, y1);
}

void PS_translateo(double theta)
{
	double xx,yy,x1,y1,t;
	xx = ((0.0-xw1)/(xw2-xw1)*(xv2-xv1) + xv1)*xsize;
	yy = ((0.0-yw1)/(yw2-yw1)*(yv2-yv1) + yv1)*ysize;
	t=theta*3.14159/180.0;
	x1 = xx*cos(t)-yy*sin(t);
	y1 = xx*sin(t)+yy*cos(t);
	fprintf(stream, "%lf cm %lf cm translate\n", xx-x1, yy-y1);
}

void PS_triangle(double x1, double y1, double d1, double a1)
{
	double r1,aa1,aa2,aa3;

	r1=d1/sqrt(3.0);
	aa1=(a1-30.0)*3.14159/180.0;
	aa2=aa1+2.0*3.14159/3.0;
	aa3=aa2+2.0*3.14159/3.0;
	PS_newpath();
	PS_plot(x1+r1*cos(aa1),y1+r1*sin(aa1),3);
	PS_plot(x1+r1*cos(aa2),y1+r1*sin(aa2),2);
	PS_plot(x1+r1*cos(aa3),y1+r1*sin(aa3),2);
	PS_closepath();
	return;
}
#endif
