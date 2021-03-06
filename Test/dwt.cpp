/**
 *  dwt97.c - Fast discrete biorthogonal CDF 9/7 wavelet forward and inverse transform (lifting implementation)
 *
 *  This code is provided "as is" and is given for educational purposes.
 *  2006 - Gregoire Pau - gregoire.pau@ebi.ac.uk
 */

#include <stdio.h>
#include <stdlib.h>
#include "QPULib.h"
#include <math.h>
//double *tempbank=0;

#define ROWS 64
#define COLS 64




void coldwt(Ptr<Float> im, Ptr<Float> tempbank){ //n is the current col number
  float a;
  int i;

  Int inc = numQPUs() << 4;
  Ptr<Float> row = im + index() + (me() << 4);
  Ptr<Float> tempRow = tempbank + index() + (me() << 4);
  // Predict 1
  a=-1.586134342;
  Float i0, i1, i2;
  For (Int j=1,j<ROWS-2,j=j+2)
    gather(row+(j-1)*inc);
    gather(row+(j+0)*inc);
    gather(row+(j+1)*inc);
    receive(i0);receive(i1);receive(i2);
    store(i1+a*(i0+i2),row+(j+0)*inc);
  End
  gather(row+(ROWS-1)*inc);
  gather(row+(ROWS-2)*inc);
  receive(i0); receive(i1);
  store(i0+2*a*i1,row+(ROWS-1)*inc);


  // Update 1
  a=-0.05298011854;
  For (Int j=2,j<ROWS,j=j+2)
    gather(row+(j-1)*inc);
    gather(row+(j+0)*inc);
    gather(row+(j+1)*inc);
    receive(i0);receive(i1);receive(i2);
    store(i1+a*(i0+i2),row+(j+0)*inc);
  End
  gather(row+(0)*inc);
  gather(row+(1)*inc);
  receive(i0);receive(i1);
  store(i0+2*a*i1,row+(0)*inc);

  // Predict 2
  a=0.8829110762;
  For (Int j=1,j<ROWS-2,j=j+2)
    gather(row+(j-1)*inc);
    gather(row+(j+0)*inc);
    gather(row+(j+1)*inc);
    receive(i0);receive(i1);receive(i2);
    store(i1+a*(i0+i2),row+(j+0)*inc);

  End
  gather(row+(ROWS-1)*inc);
  gather(row+(ROWS-2)*inc);
  receive(i0); receive(i1);
  store(i0+2*a*i1,row+(ROWS-1)*inc);

  // Update 2
  a=0.4435068522;
  for (i=2;i<ROWS;i+=2) {
    gather(row+(i-1)*inc);
    gather(row+(i+0)*inc);
    gather(row+(i+1)*inc);
    receive(i0);receive(i1);receive(i2);
    store(i1+a*(i0+i2),row+(i+0)*inc);
  }
  gather(row+(0)*inc);
  gather(row+(1)*inc);
  receive(i0);receive(i1);
  store(i0+2*a*i1,row+(0)*inc);

  // Scale
  a=1/1.149604398;
  float b=1.149604398;
  for (i=0;i<ROWS;i++) {
    gather(row+(i)*inc);
    receive(i0);
    if (i%2) store(i0*a,row+(i)*inc);
    else store(i0*b,row+(i)*inc);
  }

  // Pack
  //if (tempbank==0) tempbank=(double *)malloc(n*sizeof(double));
  int count = 0;
  for (i=0;i<ROWS;i++) {
    gather(row+(i)*inc);
    receive(i0);
    if (i%2==0) store(i0,tempRow+(i/2)*inc);
    else {
      store(i0,tempRow+(32+count)*inc);
      count++;
    }
  }
  for (i=0;i<ROWS;i++){
    gather(tempRow+(i)*inc);
    receive(i0);
    store(i0,row+(i)*inc);
  }

}




/**
 *  fwt97 - Forward biorthogonal 9/7 wavelet transform (lifting implementation)
 *
 *  x is an input signal, which will be replaced by its output transform.
 *  n is the length of the signal, and must be a power of 2.
 *
 *  The first half part of the output signal contains the approximation coefficients.
 *  The second half part contains the detail coefficients (aka. the wavelets coefficients).
 *
 *  See also iwt97.
 */

/**
void rowdwt(Float) { //n is the current row number
  float a;
  int i;

  // Predict 1
  a=-1.586134342;
  for (i=1;i<COLS-2;i+=2) {
    x[n][i]+=a*(x[n][i-1]+x[n][i+1]);
  }
  x[n][COLS-1]+=2*a*x[n][COLS-2];

  // Update 1
  a=-0.05298011854;
  for (i=2;i<COLS;i+=2) {
    x[n][i]+=a*(x[n][i-1]+x[n][i+1]);
  }
  x[n][0]+=2*a*x[n][1];

  // Predict 2
  a=0.8829110762;
  for (i=1;i<COLS-2;i+=2) {
    x[n][i]+=a*(x[n][i-1]+x[n][i+1]);
  }
  x[n][COLS-1]+=2*a*x[n][COLS-2];

  // Update 2
  a=0.4435068522;
  for (i=2;i<COLS;i+=2) {
    x[n][i]+=a*(x[n][i-1]+x[n][i+1]);
  }
  x[n][0]+=2*a*x[n][1];

  // Scale
  a=1/1.149604398;
  for (i=0;i<COLS;i++) {
    if (i%2) x[n][i]*=a;
    else x[n][i]/=a;
  }

  // Pack
  //if (tempbank==0) tempbank=(double *)malloc(n*sizeof(double));
  for (i=0;i<COLS;i++) {
    if (i%2==0) tempbank[i/2]=x[n][i];
    else tempbank[COLS/2+i/2]=x[n][i];
  }
  for (i=0;i<COLS;i++) x[n][i]=tempbank[i];
}

*/


/**
 *  iwt97 - Inverse biorthogonal 9/7 wavelet transform
 *
 *  This is the inverse of fwt97 so that iwt97(fwt97(x,n),n)=x for every signal x of length n.
 *
 *  See also fwt97.
 */
/**
void rowiwt(int n) { //n is the current row
  double a;
  int i;

  // Unpack
  //if (tempbank==0) tempbank=(double *)malloc(n*sizeof(double));
  for (i=0;i<COLS/2;i++) {
    tempbank[i*2]=x[n][i];
    tempbank[i*2+1]=x[n][i+COLS/2];
  }
  for (i=0;i<COLS;i++) x[n][i]=tempbank[i];

  // Undo scale
  a=1.149604398;
  for (i=0;i<COLS;i++) {
    if (i%2) x[n][i]*=a;
    else x[n][i]/=a;
  }

  // Undo update 2
  a=-0.4435068522;
  for (i=2;i<COLS;i+=2) {
    x[n][i]+=a*(x[n][i-1]+x[n][i+1]);
  }
  x[n][0]+=2*a*x[n][1];

  // Undo predict 2
  a=-0.8829110762;
  for (i=1;i<COLS-2;i+=2) {
    x[n][i]+=a*(x[n][i-1]+x[n][i+1]);
  }
  x[n][COLS-1]+=2*a*x[n][COLS-2];

  // Undo update 1
  a=0.05298011854;
  for (i=2;i<COLS;i+=2) {
    x[n][i]+=a*(x[n][i-1]+x[n][i+1]);
  }
  x[n][0]+=2*a*x[n][1];

  // Undo predict 1
  a=1.586134342;
  for (i=1;i<COLS-2;i+=2) {
    x[n][i]+=a*(x[n][i-1]+x[n][i+1]);
  }
  x[n][COLS-1]+=2*a*x[n][COLS-2];
}
*/
/**
void coliwt(int n){
    double a;
    int i;

    // Unpack
    //if (tempbank==0) tempbank=(double *)malloc(n*sizeof(double));
    for (i=0;i<ROWS/2;i++) {
      tempbank[i*2]=x[i][n];
      tempbank[i*2+1]=x[i+ROWS/2][n];
    }
    for (i=0;i<ROWS;i++) x[i][n]=tempbank[i];

    // Undo scale
    a=1.149604398;
    for (i=0;i<ROWS;i++) {
      if (i%2) x[i][n]*=a;
      else x[i][n]/=a;
    }

    // Undo update 2
    a=-0.4435068522;
    for (i=2;i<ROWS;i+=2) {
      x[i][n]+=a*(x[i-1][n]+x[i+1][n]);
    }
    x[0][n]+=2*a*x[1][n];

    // Undo predict 2
    a=-0.8829110762;
    for (i=1;i<ROWS-2;i+=2) {
      x[i][n]+=a*(x[i-1][n]+x[i+1][n]);
    }
    x[ROWS-1][n]+=2*a*x[ROWS-2][n];

    // Undo update 1
    a=0.05298011854;
    for (i=2;i<ROWS;i+=2) {
      x[i][n]+=a*(x[i-1][n]+x[i+1][n]);
    }
    x[0][n]+=2*a*x[1][n];

    // Undo predict 1
    a=1.586134342;
    for (i=1;i<ROWS-2;i+=2) {
      x[i][n]+=a*(x[i-1][n]+x[i+1][n]);
    }
    x[ROWS-1][n]+=2*a*x[ROWS-2][n];
}
*/



int main()
{
  // Compile the gcd function to a QPU kernel k
  auto k = compile(coldwt);

  // Allocate and initialise arrays shared between CPU and QPUs
  SharedArray<float> im(64*64);
  SharedArray<float> tempbank(64*64);



  // Set the number of QPUs to use
  k.setNumQPUs(4);

  // Invoke the kernel

  int i;

  // Makes a fancy cubic signal
  //for (i=0;i<32;i++) im[i*64]=5+i+0.4*i*i-0.02*i*i*i; //rows

  for (i=0;i<32;i++) im[i*64]=5+i+0.4*i*i-0.02*i*i*i; //cols

  // Prints original sigal x
  printf("Original signal:\n");
  //for (i=0;i<32;i++) printf("x[%d]=%f\n",i,x[0][i]); //rows
  for (i=0;i<32;i++) printf("x[%d]=%f\n",i,im[i*64]); //cols
  printf("\n");

  // Do the forward 9/7 transform
  //rowdwt(0);
  k(&im,&tempbank);

//coldwt(&im,&tempbank);

  // Prints the wavelet coefficients
  printf("Wavelets coefficients:\n");
  //for (i=0;i<32;i++) printf("wc[%d]=%f\n",i,x[0][i]); //rows
  for (i=0;i<32;i++) printf("wc[%d]=%f\n",i,im[i*64]); //cols
  printf("\n");

  // Do the inverse 9/7 transform
  //rowiwt(0);
  //coliwt(&im,&tempbank);

  // Prints the reconstructed signal
  printf("Reconstructed signal:\n");
  //for (i=0;i<32;i++) printf("xx[%d]=%f\n",i,x[0][i]); //rows
  for (i=0;i<32;i++) printf("xx[%d]=%f\n",i,im[i*64]); //cols






  // Display the result

  return 0;
}




/**
int main() {
  int i;

  // Makes a fancy cubic signal
  //for (i=0;i<32;i++) x[0][i]=5+i+0.4*i*i-0.02*i*i*i; //rows

  for (i=0;i<32;i++) x[i][0]=5+i+0.4*i*i-0.02*i*i*i; //cols

  // Prints original sigal x
  printf("Original signal:\n");
  //for (i=0;i<32;i++) printf("x[%d]=%f\n",i,x[0][i]); //rows
  for (i=0;i<32;i++) printf("x[%d]=%f\n",i,x[i][0]); //cols
  printf("\n");

  // Do the forward 9/7 transform
  //rowdwt(0);
  coldwt(0);

  // Prints the wavelet coefficients
  printf("Wavelets coefficients:\n");
  //for (i=0;i<32;i++) printf("wc[%d]=%f\n",i,x[0][i]); //rows
  for (i=0;i<32;i++) printf("wc[%d]=%f\n",i,x[i][0]); //cols
  printf("\n");

  // Do the inverse 9/7 transform
  //rowiwt(0);
  coldiwt(0);

  // Prints the reconstructed signal
  printf("Reconstructed signal:\n");
  //for (i=0;i<32;i++) printf("xx[%d]=%f\n",i,x[0][i]); //rows
  for (i=0;i<32;i++) printf("xx[%d]=%f\n",i,x[i][0]); //cols
}
*/
