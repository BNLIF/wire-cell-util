#include "WireCellUtil/Array.h"

#include <unsupported/Eigen/FFT>

#include <algorithm>
#include <complex>

using namespace WireCell;
using namespace WireCell::Array;



//http://stackoverflow.com/a/33636445

WireCell::Array::array_xxc WireCell::Array::dft(const WireCell::Array::array_xxf& arr)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    Eigen::FFT< float > fft;
    Eigen::MatrixXcf matc(nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXcf fspec(ncols); // frequency spectrum 
	// fft wants vectors, also input arr is const
	Eigen::VectorXf tmp = arr.row(irow);
	fft.fwd(fspec, tmp);
        matc.row(irow) = fspec;
    }

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows); // periodicity spectrum
        fft.fwd(pspec, matc.col(icol));
        matc.col(icol) = pspec;
    }

    return matc;
}

// WireCell::Array::const_shared_array_xxc WireCell::Array::dftptr(const WireCell::Array::array_xxf& arr)
// {
//     shared_array_xxc ret = std::make_shared<array_xxc> (nrows, ncols);
//     (*ret) = dft(arr);
//     return ret;
// }

//WireCell::Array::const_shared_array_xxf WireCell::Array::idft(const WireCell::Array::array_xxc& arr)
WireCell::Array::array_xxf WireCell::Array::idft(const WireCell::Array::array_xxc& arr)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    // fft works on matrices, not arrays, also don't step on const input
    Eigen::MatrixXcf partial(nrows, ncols);
    partial = arr.matrix();

    Eigen::FFT< float > fft;

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows); // wire spectrum
        fft.inv(pspec, partial.col(icol));
        partial.col(icol) = pspec;
    }

    //shared_array_xxf ret = std::make_shared<array_xxf> (nrows, ncols);
    array_xxf ret(nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXf wave(ncols); // back to real-valued time series
        fft.inv(wave, partial.row(irow));
        ret.row(irow) = wave;
    }

    return ret;
}

#ifdef WCT_HACK_FOR_FFTW_NO_SP

WireCell::Array::array_xxc WireCell::Array::dftd(const WireCell::Array::array_xxf& arrf)
{
    const Eigen::MatrixXd arr = arrf.cast< double >();

    const int nrows = arr.rows();
    const int ncols = arr.cols();

    Eigen::FFT< double > fft;
    Eigen::MatrixXcd matcd(nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXcd fspec(ncols); // frequency spectrum 
	// fft wants vectors, also input arr is const
	Eigen::VectorXd tmp = arr.row(irow);
	fft.fwd(fspec, tmp);
        matcd.row(irow) = fspec;
    }

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcd pspec(nrows); // periodicity spectrum
        fft.fwd(pspec, matcd.col(icol));
        matcd.col(icol) = pspec;
    }

    Eigen::MatrixXcf matc = matcd.cast< std::complex<float> >();
    return matc;
}

WireCell::Array::array_xxf WireCell::Array::idftd(const WireCell::Array::array_xxc& arrf)
{
    const Eigen::MatrixXcd arr = arrf.cast< std::complex<double> >();

    const int nrows = arr.rows();
    const int ncols = arr.cols();

    // fft works on matrices, not arrays, also don't step on const input
    Eigen::MatrixXcd partial(nrows, ncols);
    partial = arr.matrix();

    Eigen::FFT< double > fft;

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcd pspec(nrows); // wire spectrum
        fft.inv(pspec, partial.col(icol));
        partial.col(icol) = pspec;
    }

    array_xxf ret(nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXd wave(ncols); // back to real-valued time series
        fft.inv(wave, partial.row(irow));
        ret.row(irow) = wave.cast< float >();
    }

    return ret;
}

#endif

// this is a cut-and-paste mashup of dft() and idft() in order to avoid temporaries.
WireCell::Array::array_xxf
WireCell::Array::deconv(const WireCell::Array::array_xxf& arr,
			const WireCell::Array::array_xxc& filter)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    Eigen::FFT< float > fft;

    Eigen::MatrixXcf matc(nrows, ncols);
    for (int irow = 0; irow < nrows; ++irow) {
	Eigen::VectorXcf fspec(ncols); // frequency spectrum 
	// fft wants vectors, also input arr is const
	Eigen::VectorXf tmp = arr.row(irow);
	fft.fwd(fspec, tmp);
	matc.row(irow) = fspec;
    }
    
    for (int icol = 0; icol < ncols; ++icol) {
	Eigen::VectorXcf pspec(nrows); // periodicity spectrum
	fft.fwd(pspec, matc.col(icol));
	matc.col(icol) = pspec;
    }

    // deconvolution via multiplication in frequency space
    Eigen::MatrixXcf filt = matc.array() * filter;

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows); // wire spectrum
        fft.inv(pspec, filt.col(icol));
        filt.col(icol) = pspec;
    }

    array_xxf ret(nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXf wave(ncols); // back to real-valued time series
        fft.inv(wave, filt.row(irow));
        ret.row(irow) = wave;
    }

    return ret;
}

