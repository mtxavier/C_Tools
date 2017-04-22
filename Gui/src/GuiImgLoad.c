#include <Classes.h>
#include <StackEnv.h>
#include <Gui.h>
#include <Tools.h>

#include <stdio.h>

/*_________________________________________
 |
 | Misc.
 |_________________________________________
*/

static void setRGBAFormat(ImgFormat *r,int Rbits,int Gbits,int Bbits,int ABits) {
	int off;
	off = 0;
	r->Desc.RGBA.EndianSwap = (0!=0);
	r->Desc.RGBA.R.offset = off;
	r->Desc.RGBA.R.depth = Rbits;
	off +=Rbits;
	r->Desc.RGBA.G.offset = off;
	r->Desc.RGBA.G.depth = Gbits;
	off += Gbits;
	r->Desc.RGBA.B.offset = off;
	r->Desc.RGBA.B.depth = Bbits;
	off += Bbits;
	r->Desc.RGBA.A.offset = off;
	r->Desc.RGBA.A.depth = ABits;
	off += ABits;
	r->Desc.RGBA.Key.offset = off;
	r->Desc.RGBA.Key.depth = 0;
}
static ImgDesc *ImgDescNull() {
	static char Data[4] = {0,0,0,0};
	static ImgDesc r;
	static int Init = (0!=0);
	if (!Init) {
		r.Format.Format = C_(Img,Format.Type,RGBA);
        setRGBAFormat(&r.Format,8,8,8,8);
		r.Dim.w = 0;
		r.Dim.h = 0;
		r.Data = Data;
		r.pitch = 0;
		Init = (0==0);
	}
	return &r;
}


/*________________________________________
 |
 | Tiff format
 |________________________________________
*/

#include <tiffio.h>

typedef struct {
	uint32 ImageLength,ImageWidth,ImageDepth,PlanarConfig,StripOffsets,TileOffsets,StripByteCounts;
	uint16 BitsPerSample,DataType,*ColorMap,FillOrder,Orientation;
	int ScanLineSize;
} TiffDir;
static void LoadTiffDir(TiffDir *dir,TIFF *f) {
    TIFFGetField(f,TIFFTAG_DATATYPE,&dir->DataType);
    TIFFGetField(f,TIFFTAG_BITSPERSAMPLE,&dir->BitsPerSample);
    TIFFGetField(f,TIFFTAG_IMAGELENGTH,&dir->ImageLength);
    TIFFGetField(f,TIFFTAG_IMAGEWIDTH, &dir->ImageWidth);
    TIFFGetField(f,TIFFTAG_IMAGEDEPTH, &dir->ImageDepth);
    TIFFGetField(f,TIFFTAG_TILEOFFSETS,&dir->TileOffsets);
    TIFFGetField(f,TIFFTAG_COLORMAP,&dir->ColorMap);
    TIFFGetField(f,TIFFTAG_FILLORDER,&dir->FillOrder);
    TIFFGetField(f,TIFFTAG_ORIENTATION,&dir->Orientation);
    TIFFGetField(f,TIFFTAG_STRIPBYTECOUNTS,&dir->StripByteCounts);
    TIFFGetField(f,TIFFTAG_STRIPOFFSETS,&dir->StripOffsets);
}

static ImgDesc **rTiffLoad(TIFF *f,int num,int OrgAtBottom,int PitchAlign) {
	TiffDir dir;
	ImgDesc **pr,*r;
	int EndDir,DirNum;
	void *p;
	int row,sample,dp;
	LoadTiffDir(&dir,f);
	dir.ScanLineSize = TIFFScanlineSize(f);
	rPush(r);
	r->Dim.w = dir.ImageLength;
	r->Dim.h = dir.ImageWidth;
	r->pitch = ((dir.ScanLineSize+PitchAlign-1)/PitchAlign)*PitchAlign;
	r->Format.Format = C_(Img,Format.Type,RGBA);
    setRGBAFormat(&r->Format,8,8,8,8);
	rnPush(r->Data,TlsRoundUp(int,r->pitch*r->Dim.h));
	if (OrgAtBottom) {
		// native convention in Tiff files it seems.
		p = r->Data; 
		dp = r->pitch;
	} else {
		p = r->Data+((r->Dim.h-1)*dir.ScanLineSize);
		dp = -r->pitch;
	}
	sample = 0;
	for (row=0; row<dir.ImageWidth; row++) {
		 TIFFReadScanline(f,p,row,sample);
		p+=dp;
	}
	EndDir = TIFFLastDirectory(f);
	if (!EndDir) {
		TIFFReadDirectory(f);
		pr = rTiffLoad(f,num+1,OrgAtBottom,PitchAlign);
	} else {
		rnPush(pr,num+2);
		pr[num+1]=0;
	}
	pr[num]=r;
	return pr;
}

ImgDesc **GuiLoadTiff(char *filename,int OrgAtBottom,int PitchAlign) {
	TIFF *f;
	ImgDesc **r;
	f = TIFFOpen(filename,"r");
	if (f) {
	     r = rTiffLoad(f,0,OrgAtBottom,PitchAlign);
	     TIFFClose(f);
	} else {
		rPush(r);
		*r = 0;
	}
	return r;
}

/*_____________________________________________________
 |
 | Png format
 |_____________________________________________________
*/

#include <png.h>

ImgDesc *GuiLoadPng(FILE *f,int OrgAtBottom,int PitchAlign) {
	int Ok;
	png_structp h;
	png_infop info,endinfo;
	ImgDesc *r;
	rPush(r);
	r->Dim.w = r->Dim.h = 0;
	r->pitch = 0; r->Data = 0;
	r->Format.Format = C_(Img,Format.Type,Grey);
	r->Format.Desc.Palette.nb = 1;
	r->Format.Desc.Palette.key = 0;
	r->Format.Desc.Palette.Colors = &GuiRgbPaletteNull; 
	h = NULL; info = NULL; endinfo = NULL;
	h = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL); 
	Ok = (h!=NULL);
	if (Ok) { 
		info = png_create_info_struct(h); 
		Ok=(info!=NULL); 
		if (!Ok) {png_destroy_read_struct(&h,0,0);} 
	}
	if (Ok) { 
		endinfo = png_create_info_struct(h); 
		Ok=(endinfo!=NULL); 
		if(!Ok) {png_destroy_read_struct(&h,&info,0);}
	}
	if (Ok) {
		png_byte **rows,**pr,**er,*pd;
		char *p;
		int dp;
		struct {
			png_uint_32 w,h;
			int depth,colortype,colornb;
			png_colorp palette;
		} dir;
		if (setjmp(png_jmpbuf(h))) { // default error handling
			png_destroy_read_struct(&h,&info,&endinfo);
			return r;
		}
		png_init_io(h,f);
		png_set_sig_bytes(h,8);
		png_read_info(h,info);
		png_get_IHDR(h,info,&dir.w,&dir.h,&dir.depth,&dir.colortype,0,0,0);
		r->pitch = png_get_rowbytes(h,info);
		r->Dim.w = dir.w; 
		r->Dim.h = dir.h;
		if (dir.depth==16) {
			// 16 bits/chanel is overkill for most applications.
			png_set_strip_16(h);
			r->pitch = (r->pitch+1)>>1;
			dir.depth=8;
		}
		switch (dir.colortype) {
		case PNG_COLOR_TYPE_GRAY: // 1,2,4,8,16
			r->Format.Format = C_(Img,Format.Type,Grey);
			r->Format.Desc.Palette.nb = 1<<dir.depth;
			r->Format.Desc.Palette.key = 1<<dir.depth;
		break;
		case PNG_COLOR_TYPE_GRAY_ALPHA: // 8,16 / channel
			r->Format.Format = C_(Img,Format.Type,Grey);
			r->Format.Desc.Palette.nb = 1<<dir.depth;
			r->Format.Desc.Palette.key = 1<<dir.depth;
			png_set_strip_alpha(h);
			r->pitch = (r->pitch+1)>>1;
        break;
		case PNG_COLOR_TYPE_RGB: // 8,16 / channel
		    r->Format.Format = C_(Img,Format.Type,RGB);
            setRGBAFormat(&r->Format,dir.depth,dir.depth,dir.depth,0);
		break;
		case PNG_COLOR_TYPE_RGBA: // 8,16 / channel
		    r->Format.Format = C_(Img,Format.Type,RGBA);
            setRGBAFormat(&r->Format,dir.depth,dir.depth,dir.depth,dir.depth);
		break;
		case PNG_COLOR_TYPE_PALETTE: { // 1,2,4,8
			GuiRgbPalette *p;
			png_color *srcc;
			GuiRGBA col;
			png_byte *transp,*pt;
			int i,transnb;
			png_color_16 *transval;
		    r->Format.Format = C_(Img,Format.Type,Palette);
		    png_get_PLTE(h,info,&dir.palette,&dir.colornb);
			r->Format.Desc.Palette.nb = dir.colornb;
			r->Format.Desc.Palette.key = dir.colornb;
			r->Format.Desc.Palette.Colors = p = GuiVariablePalette(dir.depth);
			srcc = dir.palette;
            for (i=0;i<dir.colornb;i++) {
				col.r = srcc->red;
				col.g = srcc->green;
				col.b = srcc->blue;
				col.a = 255;
				Call(p,SetColor,2(i,&col));
				srcc++;
			}
			png_get_tRNS(h,info,&transp,&transnb,&transval);
			pt = transp;
			for (i=0;i<transnb;i++) {
				r->Format.Desc.Palette.key=*pt;
				srcc = dir.palette+(*pt);
				col.r = srcc->red;
				col.g = srcc->green;
				col.b = srcc->blue;
				col.a = 0;
				Call(p,SetColor,2(*pt,&col));
				pt++;
			}
		} break;
		default:
			r->Format.Format = C_(Img,Format.Type,Grey);
			r->Format.Desc.Palette.nb = 1<<dir.depth;
			r->Format.Desc.Palette.key = 1<<dir.depth;
		}
		r->pitch = ((r->pitch+PitchAlign-1)/PitchAlign)*PitchAlign;
		rnPush(r->Data,TlsRoundUp(int,(r->pitch*r->Dim.h)));
		mEnter(Env.r);
			rnPush(rows,r->Dim.h);
			if (OrgAtBottom) {
				p = r->Data+((r->Dim.h-1)*r->pitch);
				dp = -r->pitch;
			} else {
				p = r->Data;
				dp = r->pitch;
			}
			pr = rows; er = rows+r->Dim.h; pd = (png_byte *) p;
			while (pr<er) { *pr = p; p+=dp; pr++; }
		    if (setjmp(png_jmpbuf(h))) { // default error handling
			    png_destroy_read_struct(&h,&info,&endinfo);
				mLeave(Env.r);
			    return r;
		    }
		    png_set_rows(h,info,rows);
			png_read_image(h,rows);
			png_read_end(h,endinfo);
		mLeave(Env.r);
		png_destroy_read_struct(&h,&info,&endinfo);
	}
	return r;
}
ImgDesc *GuiLoadPngFile(char *filename,int OrgAtBottom,int PitchAlign) {
	FILE *f;
	char Sig[8];
	int is_png;
	ImgDesc *r;
	f = fopen(filename,"rb");
	fread(Sig,1,8,f);
	is_png = !png_sig_cmp(Sig,0,8);
	if (is_png) {
		r = GuiLoadPng(f,OrgAtBottom,PitchAlign);
	} else {
	    r = ImgDescNull();
	}
	fclose(f);
	return r;
}

/*______________________________________________
 |
 | Gif
 |______________________________________________
*/

#include <gif_lib.h>


ImgDesc **GuiLoadGif(char *filename,int OrgAtBottom,int PitchAlign) {
	GifFileType *f;
	ImgDesc **R;
	int errcode;
	f = DGifOpenFileName(filename);
	if (f) {
		char *q;
        unsigned char *Dat;
		struct SavedImage *bIm,*eIm;
		int i,dD;
		ImgDesc *r,**pR;
		DGifSlurp(f);
		i = f->ImageCount;
		rnPush(R,i+1);
		R[i]=0; pR = R;
		bIm = f->SavedImages; eIm = bIm+i;
		while (bIm<eIm) {
		    rPush(r); 
			*pR++ = r;
		    Dat = bIm->RasterBits;
		    r->Dim.w = bIm->ImageDesc.Width; r->Dim.h = bIm->ImageDesc.Height;
			/* Palette: for now, giflib doesn't allows for paletted result */ /* {
		        GifColorType *bPal,*ePal;
		        GuiRgbPalette *Pal;
				int depth;
		        depth = bIm->ImageDesc.ColorMap->BitsPerPixel;
		        r->Format.Format = C_(Img,Format.Type,Palette);
		        r->Format.Desc.Palette.nb = bIm->ImageDesc.ColorMap->ColorCount;
		        r->Format.Desc.Palette.key = 0;
		        r->Format.Desc.Palette.Colors = Pal = GuiVariablePalette(depth);
		        bPal = bIm->ImageDesc.ColorMap->Colors;
		        ePal = bPal + bIm->ImageDesc.ColorMap->ColorCount;
		        i = 0;
		        while (bPal<ePal) {
			        GuiRGBA col;
			        col.R = bPal->Red;
			        col.G = bPal->Green;
			        col.B = bPal->Blue;
			        col.A = 255;
			        Call(Pal,SetColor,2(i,col));
			        bPal++; i++;
		        }
		        r->pitch = ((r->Dim.w*depth)+7)>>3;
			} */
			{
				r->Format.Format = C_(Img,Format.Type,RGB);
				setRGBAFormat(&r->Format,8,8,8,0);
				r->pitch = (((r->Dim.w*3)+PitchAlign-1)/PitchAlign)*PitchAlign;
			}
		    rnPush(r->Data,TlsRoundUp(int,(r->pitch*r->Dim.h)));
		    if (OrgAtBottom) {
			    dD = r->pitch;
		        q = r->Data+(dD*(r->Dim.h-1));
				dD = -dD;
		    } else {
			    dD = r->pitch;
		        q = r->Data;
		    }
		    i = 0;
            while (i<r->Dim.h) {
			    unsigned char *e;
				char *dq;
				e = Dat+(r->Dim.w*3);
				dq = q;
				while (Dat<e) { *dq++ = *Dat++; }
			    q += dD; i++;
		    }
			bIm++;
		}
	    DGifCloseFile(f);
	} else {
		rPush(R);
		*R = 0;
	}
	return R;
}

/*________________________________________________________
 |
 | Jpeg
 |________________________________________________________
*/

#include <jpeglib.h>

ImgDesc *GuiLoadJpeg(char *filename,int OrgAtBottom,int PitchAlign) {
	ImgDesc *r;
    FILE *f;
	r = ImgDescNull();
	f = fopen(filename,"rb");
	if (f) {
        struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr errormgr;
		void *q;
		int dq;
		cinfo.err = jpeg_std_error(&errormgr);
		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo,f);
		jpeg_read_header(&cinfo,TRUE);
		/**/
		rPush(r);
		jpeg_start_decompress(&cinfo);
		if (cinfo.output_components==1) { // Grey scale
			r->Format.Format = C_(Img,Format.Type,Grey);
			r->Format.Desc.Palette.key =
			r->Format.Desc.Palette.nb = 1<<BITS_IN_JSAMPLE;
			r->Format.Desc.Palette.Colors = GuiNoPalette();
		} else {
			r->Format.Format = C_(Img,Format.Type,RGB);
			if (cinfo.output_components==3) {
			    setRGBAFormat(&r->Format,BITS_IN_JSAMPLE,BITS_IN_JSAMPLE,BITS_IN_JSAMPLE,0);
			} else {
			    setRGBAFormat(&r->Format,BITS_IN_JSAMPLE,BITS_IN_JSAMPLE,BITS_IN_JSAMPLE,BITS_IN_JSAMPLE);
			}
		}
		r->Dim.w = cinfo.output_width;
		r->Dim.h = cinfo.output_height;
		r->pitch = r->Dim.w * cinfo.output_components*TlsAtomSize(JSAMPLE);
		r->pitch = (((r->pitch+PitchAlign-1)/PitchAlign)*PitchAlign);
		rnPush(r->Data,TlsRoundUp(int,r->pitch*r->Dim.h));
		if (OrgAtBottom) {
            q = r->Data+((r->Dim.h-1)*r->pitch);
			dq  = -r->pitch;
		} else {
			q = r->Data; dq = r->pitch;
		}
		/**/
        while (cinfo.output_scanline<cinfo.output_height) {
			JSAMPROW ln;
			JSAMPARRAY dst;
			int nbr;
			ln = q; *dst = ln;
			nbr = jpeg_read_scanlines(&cinfo,dst,1);
			if (nbr) q+=r->pitch;
		}
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
	    fclose(f);
	}
	return r;
}

/*________________________________________________________
 |
 | Jpeg
 |________________________________________________________
*/

static int extDiff(char *p,char *q) {
	int r;
	while ((*p)&&(*p==*q)) { p++; q++; }
	return (*p!=*q);
}

ImgDesc **GuiLoadImage(char *fname,int OrgAtBottom,int PitchAlign) {
	ImgDesc **r;
	char ext[6],*p,*q,*e;
	int fail,select;
	e = fname; ext[5] = 0;
	while (*e) e++;
	q = ext+4; e--;
	while ((q>=ext)&&(e>=fname)) { char c; c=*e; if ((c>='A')&&(c<='Z')){c=c+('a'-'A');} *q=c; q--; e--; }
	while (q>=ext) { *q=' '; q--;}
	fail = ((ext[0]!='.')&&(ext[1]!='.'));
	fail = fail&&((ext[4]!='g')&&(ext[4]!='f'));
	if (!fail) {
		if (ext[4]=='g') {
			if (ext[3]=='n') {
                select = 0;
				fail = extDiff(".png",ext+1);
			} else {
				select = 1;
				fail = (extDiff(".jpg",ext+1) && extDiff(".jpeg",ext));
			}
		} else {
			if (ext[2]=='g') {
				select = 2;
				fail = extDiff(".gif",ext+1);
			} else {
				select = 3;
				fail = (extDiff(".tif",ext+1) && extDiff(".tiff",ext));
			}
		}
	}
	if (fail) {
		rPush(r);
		*r = 0;
	} else {
		switch(select) {
        case 0:
			rnPush(r,2);
			r[1] = 0;
            r[0] = GuiLoadPngFile(fname,OrgAtBottom,PitchAlign);
		break;
		case 1:
		    rnPush(r,2);
			r[1] = 0;
			r[0] = GuiLoadJpeg(fname,OrgAtBottom,PitchAlign);
		break;
		case 2:
		    r = GuiLoadGif(fname,OrgAtBottom,PitchAlign);
		break;
		case 3:
            r = GuiLoadTiff(fname,OrgAtBottom,PitchAlign);
		break;
		}
	}
	return r;
}

