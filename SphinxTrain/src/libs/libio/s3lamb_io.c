/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*********************************************************************
 *
 * File: s3lamb_io.c
 * 
 * Description: 
 *    Read/write interpolation lambda files
 *
 * Author: 
 *    Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s3lamb_io.h>
#include <s3/s3io.h>
#include <s3/s3.h>

#include <string.h>

int
s3lamb_read(const char *fn,
	    float32 **out_lambda,
	    uint32 *out_n_lambda)
{
    FILE *fp;
    const char *ver;
    const char *do_chk;
    uint32 rd_chksum = 0, sv_chksum, ignore;
    float32 *lambda;
    uint32 n_lambda;
    uint32 swap;
    
    fp = s3open(fn, "rb", &swap);
    if (fp == NULL)
	return S3_ERROR;

    /* check version io */
    ver = s3get_gvn_fattr("version");
    if (ver) {
	if (strcmp(ver, LAMBDA_FILE_VERSION) != 0) {
	    E_FATAL("Version mismatch for %s, file ver: %s != reader ver: %s\n",
		    fn, ver, LAMBDA_FILE_VERSION);
	}
    }
    else {
	E_FATAL("No version attribute for %s\n", fn);
    }
    
    /* if do_chk is non-NULL, there is a checksum after the data in the file */
    do_chk = s3get_gvn_fattr("chksum0");

    if (s3read_1d((void **)&lambda,
		  sizeof(float32),
		  &n_lambda,
		  fp,
		  swap,
		  &rd_chksum) != S3_SUCCESS)
	return S3_ERROR;

    if (do_chk) {
	/* See if the checksum in the file matches that which
	   was computed from the read data */

	if (s3read(&sv_chksum, sizeof(uint32), 1, fp, swap, &ignore) != 1) {
	    s3close(fp);

	    return S3_ERROR;
	}

	if (sv_chksum != rd_chksum) {
	    E_FATAL("Checksum error; read corrupt data.\n");
	}
    }

    s3close(fp);

    *out_lambda = lambda;
    *out_n_lambda = n_lambda;

    E_INFO("Read %s [%u array]\n", fn, n_lambda);

    return S3_SUCCESS;
}

int
s3lamb_write(const char *fn,
	     const float32 *lambda,
	     uint32 n_lambda)
{
    FILE *fp;
    uint32 chksum = 0;
    uint32 ignore = 0;

    s3clr_fattr();
    s3add_fattr("version", LAMBDA_FILE_VERSION, TRUE);
    s3add_fattr("chksum0", "yes", TRUE);


    fp = s3open(fn, "wb", NULL);
    if (fp == NULL)
	return S3_ERROR;

    if (s3write_1d((void *)lambda, sizeof(float32), n_lambda, fp, &chksum) != S3_SUCCESS) {
	s3close(fp);
	return S3_ERROR;
    }

    if (s3write(&chksum, sizeof(uint32), 1, fp, &ignore) != 1) {
	s3close(fp);

	return S3_ERROR;
    }

    s3close(fp);

    E_INFO("Wrote %s [%u array]\n", fn, n_lambda);

    return S3_SUCCESS;
}

int
s3lambcnt_read(const char *fn,
	       uint32 **out_lambda_cnt,
	       uint32 *out_n_lambda_cnt)
{
    FILE *fp;
    const char *ver;
    const char *do_chk;
    uint32 rd_chksum = 0, sv_chksum, ignore;
    uint32 *lambda_cnt;
    uint32 n_lambda_cnt;
    uint32 swap;

    fp = s3open(fn, "rb", &swap);
    if (fp == NULL)
	return S3_ERROR;

    /* check version io */
    ver = s3get_gvn_fattr("version");
    if (ver) {
	if (strcmp(ver, LAMBDACNT_FILE_VERSION) != 0) {
	    E_FATAL("Version mismatch for %s, file ver: %s != reader ver: %s\n",
		    fn, ver, LAMBDACNT_FILE_VERSION);
	}
    }
    else {
	E_FATAL("No version attribute for %s\n", fn);
    }
    
    /* if do_chk is non-NULL, there is a checksum after the data in the file */
    do_chk = s3get_gvn_fattr("chksum0");

    if (s3read_1d((void **)&lambda_cnt,
		  sizeof(uint32),
		  &n_lambda_cnt,
		  fp,
		  swap,
		  &rd_chksum) != S3_SUCCESS)
	return S3_ERROR;

    if (do_chk) {
	/* See if the checksum in the file matches that which
	   was computed from the read data */

	if (s3read(&sv_chksum, sizeof(uint32), 1, fp, swap, &ignore) != 1) {
	    s3close(fp);

	    return S3_ERROR;
	}

	if (sv_chksum != rd_chksum) {
	    E_FATAL("Checksum error; read corrupt data.\n");
	}
    }

    s3close(fp);

    *out_lambda_cnt = lambda_cnt;
    *out_n_lambda_cnt = n_lambda_cnt;

    E_INFO("Read %s [%u array]\n", fn, n_lambda_cnt);

    return S3_SUCCESS;
}

int
s3lambcnt_write(const char *fn,
		const uint32 *lambda_cnt,
		uint32 n_lambda_cnt)
{
    FILE *fp;
    uint32 chksum = 0;
    uint32 ignore = 0;

    s3clr_fattr();
    s3add_fattr("version", LAMBDACNT_FILE_VERSION, TRUE);
    s3add_fattr("chksum0", "yes", TRUE);


    fp = s3open(fn, "wb", NULL);
    if (fp == NULL)
	return S3_ERROR;

    if (s3write_1d((void *)lambda_cnt, sizeof(uint32), n_lambda_cnt, fp, &chksum) != S3_SUCCESS) {
	s3close(fp);
	return S3_ERROR;
    }

    if (s3write(&chksum, sizeof(uint32), 1, fp, &ignore) != 1) {
	s3close(fp);

	return S3_ERROR;
    }

    s3close(fp);

    E_INFO("Wrote %s [%u array]\n", fn, n_lambda_cnt);

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:05:40  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
