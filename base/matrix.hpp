

void MatrixOrthoLH(CMatrix *m,float w,float h,float zn,float zf)
{
	m->a[0][0]=2.0f/w;		m->a[0][1]=0.0f;		m->a[0][2]=0.0f;			m->a[0][3]=0.0f;
	m->a[1][0]=0.0f;		m->a[1][1]=2.0f/h;		m->a[1][2]=0.0f;			m->a[1][3]=0.0f;
	m->a[2][0]=0.0f;		m->a[2][1]=0.0f;		m->a[2][2]=1.0f/(zf-zn);	m->a[2][3]=0.0f;
	m->a[3][0]=0.0f;		m->a[3][1]=0.0f;		m->a[3][2]=zn/(zn-zf);		m->a[3][3]=1.0f;
}

void MatrixOrthoLHS(CMatrix *m,float w,float h,float zn,float zf)
{
    m->a[0][0]=2.0f/w;      m->a[0][1]=0.0f;        m->a[0][2]=0.0f;            m->a[0][3]=0.0f;
    m->a[1][0]=0.0f;        m->a[1][1]=-2.0f/h;     m->a[1][2]=0.0f;            m->a[1][3]=0.0f;
    m->a[2][0]=0.0f;        m->a[2][1]=0.0f;        m->a[2][2]=1.0f/(zf-zn);    m->a[2][3]=0.0f;
    m->a[3][0]=0.0f;        m->a[3][1]=0.0f;        m->a[3][2]=zn/(zn-zf);      m->a[3][3]=1.0f;
}

void MatrixPerspectiveFovLH(CMatrix *m,float fovy,float Aspect,float zn,float zf)
{
	float yScale=1.0f/tanf(fovy/2);
	float xScale=yScale/Aspect;
	m->a[0][0]=xScale;		m->a[0][1]=0.0f;		m->a[0][2]=0.0f;			m->a[0][3]=0.0f;
	m->a[1][0]=0.0f;		m->a[1][1]=yScale;		m->a[1][2]=0.0f;			m->a[1][3]=0.0f;
	m->a[2][0]=0.0f;		m->a[2][1]=0.0f;		m->a[2][2]=zf/(zf-zn);		m->a[2][3]=1.0f;
	m->a[3][0]=0.0f;		m->a[3][1]=0.0f;		m->a[3][2]=-zn*zf/(zf-zn);	m->a[3][3]=0.0f;
}

