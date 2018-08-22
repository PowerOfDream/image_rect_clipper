#include <fstream>
#include <assert.h>

static void diagonal(double** a, int N, double* b) {
	int i, j, k;
	double temp = 0;

	for (i = 0; i < N; i++) {
		if (a[i][i] == 0) {
			for (j = 0; j < N; j++) {
				if (j == i) continue;
				if (a[j][i] != 0 && a[i][j] != 0) {
					for (k = 0; k < N; k++) {
						temp = a[j][k];
						a[j][k] = a[i][k];
						a[i][k] = temp;
					}
					temp = b[j];
					b[j] = b[i];
					b[i] = temp;
					break;
				}
			}
		}
	}
}

int do_matrix_solve(double** a, int N, double* b, double*x)
{
	memset(x, 0, sizeof(double)* N);

	diagonal(a, N, b);

	int i, j, k;
	for (k = 0; k < N; k++) {
		for (i = k + 1; i < N; i++) {
			if (a[k][k] == 0) {
				//Solution is not exist!
				return 0;
			}
			double M = a[i][k] / a[k][k];
			for (j = k; j < N; j++) {
				a[i][j] -= M * a[k][j];
			}
			b[i] -= M*b[k];
		}
	}
	for (i = N - 1; i >= 0; i--) {
		double s = 0;
		for (j = i; j < N; j++) {
			s = s + a[i][j] * x[j];
		}
		x[i] = (b[i] - s) / a[i][i];
	}

	return 1;
}

//将源图像中的一个任意四边形转变换一个正矩形目标图像
//成功返回1
//失败返回0
int  image_trans(unsigned char*  psrc,       //原始图像
	             int             src_w,
	             int             src_h,

	             int             src_x[4],   //原始图像中的四边形的角点坐标，按逆时针排列
	             int             src_y[4],

	             unsigned char*  pdst,       //目标图像
	             int             dst_w,
	             int             dst_h)
{
	double A[4][4] = { 0 };
	double Ks[4]   = { 0 };
	double Kt[4]   = { 0 };
	double Bs[4]   = { 0 };
	double Bt[4]   = { 0 };

	double Mx[4] = { 1.0, double(dst_w), double(dst_w),          1.0  };
	double My[4] = { 1.0,           1.0, double(dst_h), double(dst_h) };

	for (int i = 0; i < 4; i++) {
		A[i][0] = Mx[i];
		A[i][1] = My[i];
		A[i][2] = Mx[i] * My[i];
		A[i][3] = 1.0;

		Bs[i] = src_x[i];
		Bt[i] = src_y[i];
	}

	//解矩阵 A*Ks = Bs , A*Kt = Bt
	double B[4][4];
	memcpy(B, A, sizeof(A));
	double* Temp1[4] = { &A[0][0], &A[1][0], &A[2][0], &A[3][0] };
	double* Temp2[4] = { &B[0][0], &B[1][0], &B[2][0], &B[3][0] };
	int r1 = do_matrix_solve(Temp1, 4, Bs, Ks);
	int r2 = do_matrix_solve(Temp2, 4, Bt, Kt);
	assert(r1 && r2);
	if (!r1 || !r2)
		return 0;

	//空间变换+双线性插值
	unsigned char* t = pdst;
	for (int y = 1; y <= dst_h; y++) {
		for (int x = 1; x <= dst_w; x++) {
			double fu = Ks[0] * x + Ks[1] * y + Ks[2] * x * y + Ks[3];
			double fv = Kt[0] * x + Kt[1] * y + Kt[2] * x * y + Kt[3];
			int iu = (int)fu;
			int iv = (int)fv;
			double rx = fu - iu;
			double ry = fv - iv;

			if (iu < 0 || iu + 1 >= src_w || iv < 0 || iv + 1 >= src_h) {
				*t++ = 0;
			}
			else {
				unsigned char result = (unsigned char)(
					psrc[(iv + 0) * src_w + (iu + 0)] * (1 - rx) * (1 - ry) +
					psrc[(iv + 0) * src_w + (iu + 1)] * (rx) * (1 - ry) +
					psrc[(iv + 1) * src_w + (iu + 0)] * (1 - rx) * (ry)+
					psrc[(iv + 1) * src_w + (iu + 1)] * (rx) * (ry)+0.5);

				*t++ = result;
			}
		}
	}

	return 1;
}

//test
int main()
{
	static const int TEST_BMP_FILE_SIZE = 1556346;
	static const int src_w = 1080;
	static const int src_h = 1440;
	static const int IMG_HEAD = TEST_BMP_FILE_SIZE - src_w * src_h;
	
	int src_x[4] = {            228,             637,              923,               413};
	int src_y[4] = {src_h - 1 - 707, src_h - 1 - 623,  src_h - 1 - 827,   src_h - 1 - 1002};//BMP像素是垂直镜像的
	int dst_w = 512;
	int dst_h = 512;

	unsigned char *psrc = new unsigned char[src_w * src_h];
	unsigned char *pdst = new unsigned char[dst_w * dst_h];

	std::ifstream f("in.bmp", std::ifstream::binary);
	if (f) {
		f.seekg(IMG_HEAD);
		f.read((char*)(psrc), src_w * src_h);
		f.close();

		//将源图像中的一个任意四边形转变换一个正矩形目标图像
		image_trans(psrc, src_w, src_h, src_x, src_y, pdst, dst_w, dst_h);

		std::ofstream out("out.raw", std::ifstream::binary);
		out.write((char*)pdst, dst_w*dst_h);
		out.close();
	}

	delete[] psrc;
	delete[] pdst;

    return 0;
}

