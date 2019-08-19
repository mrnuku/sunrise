#ifndef MISCIMPORTS_H
#define MISCIMPORTS_H

// // //// // //// // //// //
/// DynamicModule
// //// // //// // //// //

class DynamicModule : public NamedObject {
private:
	WindowsNS::HMODULE									d_moduleHandle;

public:
														DynamicModule( const Str & module_name, bool drop = true );
														~DynamicModule( );

	WindowsNS::PROC										FindProc( const Str & proc_name );
};

// // //// // //// // //// //
// DynamicModule
// //// // //// // //// //

INLINE DynamicModule::DynamicModule( const Str & module_name, bool drop ) {
	d_moduleHandle = WindowsNS::LoadLibraryA( module_name.c_str( ) );
	if( !d_moduleHandle && drop )
		Common::Com_Error( ERR_DROP, "DynamicModule::DynamicModule: unable to load \"%s\"", module_name.c_str( ) );
	d_name = module_name;
}

INLINE DynamicModule::~DynamicModule( ) {
	FreeLibrary( d_moduleHandle );
}

INLINE WindowsNS::PROC DynamicModule::FindProc( const Str & proc_name ) {
	WindowsNS::PROC functionPointer = WindowsNS::GetProcAddress( d_moduleHandle, proc_name.c_str( ) );
	if( !functionPointer )
		Common::Com_Error( ERR_DROP, "DynamicModule::FindProc: unable to load \"%s\"", proc_name.c_str( ) );
	return functionPointer;
}

typedef int ( APIENTRY * ZLIBINFLATEINIT2_ )( ZLib::z_streamp strm, int windowBits, const char * version, int stream_size );
typedef int ( APIENTRY * ZLIBINFLATE )( ZLib::z_streamp strm, int flush );
typedef int ( APIENTRY * ZLIBINFLATEEND )( ZLib::z_streamp strm );
typedef ZLib::uLong ( APIENTRY * ZLIBCRC32 )( ZLib::uLong crc, const ZLib::Bytef * buf, ZLib::uInt len );

class ZLibImports {

private:

	static DynamicModule *							zlibModule;

public:

	static ZLIBINFLATEINIT2_						zlInflateInit2_;
	static ZLIBINFLATE								zlInflate;
	static ZLIBINFLATEEND							zlInflateEnd;
	static ZLIBCRC32								zlCrc32;

	static int										zlInflateInit2( ZLib::z_streamp strm, int windowBits ) { return zlInflateInit2_( strm, windowBits, ZLIB_VERSION, sizeof( ZLib::z_stream ) ); }

	static void										Init( );
	static void										Shutdown( );
};

typedef Png::png_structp ( PNGAPI * PNG_CREATE_READ_STRUCT_PROC )( Png::png_const_charp user_png_ver, Png::png_voidp error_ptr, Png::png_error_ptr error_fn, Png::png_error_ptr warn_fn );
typedef Png::png_infop ( PNGAPI * PNG_CREATE_INFO_STRUCT_PROC )( Png::png_structp png_ptr );
typedef void ( PNGAPI * PNG_SET_READ_FN_PROC )( Png::png_structp png_ptr, Png::png_voidp io_ptr, Png::png_rw_ptr read_data_fn );
typedef void ( PNGAPI * PNG_READ_INFO_PROC )( Png::png_structp png_ptr, Png::png_infop info_ptr );
typedef Png::png_uint_32 ( PNGAPI * PNG_GET_IHDR_PROC )( Png::png_structp png_ptr, Png::png_infop info_ptr, Png::png_uint_32 * width, Png::png_uint_32 * height, int * bit_depth, int * color_type, int * interlace_method, int *compression_method, int * filter_method );
typedef void ( PNGAPI * PNG_SET_EXPAND_PROC )( Png::png_structp png_ptr );
typedef void ( PNGAPI * PNG_SET_SCALE_16_PROC )( Png::png_structp png_ptr );
typedef void ( PNGAPI * PNG_READ_IMAGE_PROC )( Png::png_structp png_ptr, Png::png_bytepp image );
typedef void ( PNGAPI * PNG_READ_ROW_PROC )( Png::png_structp png_ptr, Png::png_bytep row, Png::png_bytep display_row );
typedef void ( PNGAPI * PNG_READ_END_PROC )( Png::png_structp png_ptr, Png::png_infop info_ptr );
typedef void ( PNGAPI * PNG_DESTROY_READ_STRUCT_PROC )( Png::png_structpp png_ptr_ptr, Png::png_infopp info_ptr_ptr, Png::png_infopp end_info_ptr_ptr );
typedef Png::png_structp ( PNGAPI * PNG_CREATE_WRITE_STRUCT_PROC )( Png::png_const_charp user_png_ver, Png::png_voidp error_ptr, Png::png_error_ptr error_fn, Png::png_error_ptr warn_fn );
typedef void ( PNGAPI * PNG_SET_WRITE_FN_PROC )( Png::png_structp png_ptr, Png::png_voidp io_ptr, Png::png_rw_ptr write_data_fn, Png::png_flush_ptr output_flush_fn );
typedef void ( PNGAPI * PNG_SET_COMPRESSION_LEVEL_PROC )( Png::png_structp png_ptr, int level );
typedef void ( PNGAPI * PNG_SET_COMPRESSION_MEM_LEVEL_PROC )( Png::png_structp png_ptr, int mem_level );
typedef void ( PNGAPI * PNG_SET_COMPRESSION_STRATEGY_PROC )( Png::png_structp png_ptr, int strategy );
typedef void ( PNGAPI * PNG_SET_COMPRESSION_WINDOW_BITS_PROC )( Png::png_structp png_ptr, int window_bits );
typedef void ( PNGAPI * PNG_SET_COMPRESSION_METHOD_PROC )( Png::png_structp png_ptr, int method );
typedef void ( PNGAPI * PNG_SET_COMPRESSION_BUFFER_SIZE_PROC )( Png::png_structp png_ptr, Png::png_size_t size );
typedef void ( PNGAPI * PNG_SET_IHDR_PROC )( Png::png_structp png_ptr, Png::png_infop info_ptr, Png::png_uint_32 width, Png::png_uint_32 height, int bit_depth, int color_type, int interlace_method, int compression_method, int filter_method );
typedef void ( PNGAPI * PNG_SET_PLTE_PROC )( Png::png_structp png_ptr, Png::png_infop info_ptr, Png::png_const_colorp palette, int num_palette );
typedef void ( PNGAPI * PNG_WRITE_INFO_PROC )( Png::png_structp png_ptr, Png::png_infop info_ptr );
typedef void ( PNGAPI * PNG_WRITE_ROW_PROC )( Png::png_structp png_ptr, Png::png_const_bytep row );
typedef void ( PNGAPI * PNG_WRITE_END_PROC )( Png::png_structp png_ptr, Png::png_infop info_ptr );
typedef void ( PNGAPI * PNG_DESTROY_WRITE_STRUCT_PROC )( Png::png_structpp png_ptr_ptr, Png::png_infopp info_ptr_ptr );
typedef void ( PNGAPI * PNG_ERROR_PROC )( Png::png_structp png_ptr, Png::png_const_charp error_message );
typedef Png::png_voidp ( PNGAPI * PNG_GET_IO_PTR_PROC )( Png::png_structp png_ptr );

#ifndef DEDICATED_ONLY
class PngImports {

private:

	static DynamicModule *							pngModule;

public:

	static PNG_CREATE_READ_STRUCT_PROC				png_create_read_struct;
	static PNG_CREATE_INFO_STRUCT_PROC				png_create_info_struct;
	static PNG_SET_READ_FN_PROC						png_set_read_fn;
	static PNG_READ_INFO_PROC						png_read_info;
	static PNG_GET_IHDR_PROC						png_get_IHDR;
	static PNG_SET_EXPAND_PROC						png_set_expand;
	static PNG_SET_SCALE_16_PROC					png_set_scale_16;
	static PNG_READ_IMAGE_PROC						png_read_image;
	static PNG_READ_ROW_PROC						png_read_row;
	static PNG_READ_END_PROC						png_read_end;
	static PNG_DESTROY_READ_STRUCT_PROC				png_destroy_read_struct;
	static PNG_CREATE_WRITE_STRUCT_PROC				png_create_write_struct;
	static PNG_SET_WRITE_FN_PROC					png_set_write_fn;
	static PNG_SET_COMPRESSION_LEVEL_PROC			png_set_compression_level;
	static PNG_SET_COMPRESSION_MEM_LEVEL_PROC		png_set_compression_mem_level;
	static PNG_SET_COMPRESSION_STRATEGY_PROC		png_set_compression_strategy;
	static PNG_SET_COMPRESSION_WINDOW_BITS_PROC		png_set_compression_window_bits;
	static PNG_SET_COMPRESSION_METHOD_PROC			png_set_compression_method;
	static PNG_SET_COMPRESSION_BUFFER_SIZE_PROC		png_set_compression_buffer_size;
	static PNG_SET_IHDR_PROC						png_set_IHDR;
	static PNG_SET_PLTE_PROC						png_set_PLTE;
	static PNG_WRITE_INFO_PROC						png_write_info;
	static PNG_WRITE_ROW_PROC						png_write_row;
	static PNG_WRITE_END_PROC						png_write_end;
	static PNG_DESTROY_WRITE_STRUCT_PROC			png_destroy_write_struct;
	static PNG_ERROR_PROC							png_error;
	static PNG_GET_IO_PTR_PROC						png_get_io_ptr;

	static void										Init( );
	static void										Shutdown( );
	static bool										IsAvailable( );

	static void PNGCBAPI							PngReadCallback( Png::png_structp png_ptr, Png::png_bytep data, Png::png_size_t length );
	static void PNGCBAPI							PngWriteCallback( Png::png_structp png_ptr, Png::png_bytep data, Png::png_size_t length );
	static void PNGCBAPI							PngFlushCallback( Png::png_structp png_ptr );
};

namespace WindowsNS {

enum DWRITE_FONT_FEATURE_TAG {

	DWRITE_FONT_FEATURE_TAG_ALTERNATIVE_FRACTIONS               = 0x63726661, // 'afrc'
	DWRITE_FONT_FEATURE_TAG_PETITE_CAPITALS_FROM_CAPITALS       = 0x63703263, // 'c2pc'
	DWRITE_FONT_FEATURE_TAG_SMALL_CAPITALS_FROM_CAPITALS        = 0x63733263, // 'c2sc'
	DWRITE_FONT_FEATURE_TAG_CONTEXTUAL_ALTERNATES               = 0x746c6163, // 'calt'
	DWRITE_FONT_FEATURE_TAG_CASE_SENSITIVE_FORMS                = 0x65736163, // 'case'
	DWRITE_FONT_FEATURE_TAG_GLYPH_COMPOSITION_DECOMPOSITION     = 0x706d6363, // 'ccmp'
	DWRITE_FONT_FEATURE_TAG_CONTEXTUAL_LIGATURES                = 0x67696c63, // 'clig'
	DWRITE_FONT_FEATURE_TAG_CAPITAL_SPACING                     = 0x70737063, // 'cpsp'
	DWRITE_FONT_FEATURE_TAG_CONTEXTUAL_SWASH                    = 0x68777363, // 'cswh'
	DWRITE_FONT_FEATURE_TAG_CURSIVE_POSITIONING                 = 0x73727563, // 'curs'
	DWRITE_FONT_FEATURE_TAG_DEFAULT                             = 0x746c6664, // 'dflt'
	DWRITE_FONT_FEATURE_TAG_DISCRETIONARY_LIGATURES             = 0x67696c64, // 'dlig'
	DWRITE_FONT_FEATURE_TAG_EXPERT_FORMS                        = 0x74707865, // 'expt'
	DWRITE_FONT_FEATURE_TAG_FRACTIONS                           = 0x63617266, // 'frac'
	DWRITE_FONT_FEATURE_TAG_FULL_WIDTH                          = 0x64697766, // 'fwid'
	DWRITE_FONT_FEATURE_TAG_HALF_FORMS                          = 0x666c6168, // 'half'
	DWRITE_FONT_FEATURE_TAG_HALANT_FORMS                        = 0x6e6c6168, // 'haln'
	DWRITE_FONT_FEATURE_TAG_ALTERNATE_HALF_WIDTH                = 0x746c6168, // 'halt'
	DWRITE_FONT_FEATURE_TAG_HISTORICAL_FORMS                    = 0x74736968, // 'hist'
	DWRITE_FONT_FEATURE_TAG_HORIZONTAL_KANA_ALTERNATES          = 0x616e6b68, // 'hkna'
	DWRITE_FONT_FEATURE_TAG_HISTORICAL_LIGATURES                = 0x67696c68, // 'hlig'
	DWRITE_FONT_FEATURE_TAG_HALF_WIDTH                          = 0x64697768, // 'hwid'
	DWRITE_FONT_FEATURE_TAG_HOJO_KANJI_FORMS                    = 0x6f6a6f68, // 'hojo'
	DWRITE_FONT_FEATURE_TAG_JIS04_FORMS                         = 0x3430706a, // 'jp04'
	DWRITE_FONT_FEATURE_TAG_JIS78_FORMS                         = 0x3837706a, // 'jp78'
	DWRITE_FONT_FEATURE_TAG_JIS83_FORMS                         = 0x3338706a, // 'jp83'
	DWRITE_FONT_FEATURE_TAG_JIS90_FORMS                         = 0x3039706a, // 'jp90'
	DWRITE_FONT_FEATURE_TAG_KERNING                             = 0x6e72656b, // 'kern'
	DWRITE_FONT_FEATURE_TAG_STANDARD_LIGATURES                  = 0x6167696c, // 'liga'
	DWRITE_FONT_FEATURE_TAG_LINING_FIGURES                      = 0x6d756e6c, // 'lnum'
	DWRITE_FONT_FEATURE_TAG_LOCALIZED_FORMS                     = 0x6c636f6c, // 'locl'
	DWRITE_FONT_FEATURE_TAG_MARK_POSITIONING                    = 0x6b72616d, // 'mark'
	DWRITE_FONT_FEATURE_TAG_MATHEMATICAL_GREEK                  = 0x6b72676d, // 'mgrk'
	DWRITE_FONT_FEATURE_TAG_MARK_TO_MARK_POSITIONING            = 0x6b6d6b6d, // 'mkmk'
	DWRITE_FONT_FEATURE_TAG_ALTERNATE_ANNOTATION_FORMS          = 0x746c616e, // 'nalt'
	DWRITE_FONT_FEATURE_TAG_NLC_KANJI_FORMS                     = 0x6b636c6e, // 'nlck'
	DWRITE_FONT_FEATURE_TAG_OLD_STYLE_FIGURES                   = 0x6d756e6f, // 'onum'
	DWRITE_FONT_FEATURE_TAG_ORDINALS                            = 0x6e64726f, // 'ordn'
	DWRITE_FONT_FEATURE_TAG_PROPORTIONAL_ALTERNATE_WIDTH        = 0x746c6170, // 'palt'
	DWRITE_FONT_FEATURE_TAG_PETITE_CAPITALS                     = 0x70616370, // 'pcap'
	DWRITE_FONT_FEATURE_TAG_PROPORTIONAL_FIGURES                = 0x6d756e70, // 'pnum'
	DWRITE_FONT_FEATURE_TAG_PROPORTIONAL_WIDTHS                 = 0x64697770, // 'pwid'
	DWRITE_FONT_FEATURE_TAG_QUARTER_WIDTHS                      = 0x64697771, // 'qwid'
	DWRITE_FONT_FEATURE_TAG_REQUIRED_LIGATURES                  = 0x67696c72, // 'rlig'
	DWRITE_FONT_FEATURE_TAG_RUBY_NOTATION_FORMS                 = 0x79627572, // 'ruby'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_ALTERNATES                = 0x746c6173, // 'salt'
	DWRITE_FONT_FEATURE_TAG_SCIENTIFIC_INFERIORS                = 0x666e6973, // 'sinf'
	DWRITE_FONT_FEATURE_TAG_SMALL_CAPITALS                      = 0x70636d73, // 'smcp'
	DWRITE_FONT_FEATURE_TAG_SIMPLIFIED_FORMS                    = 0x6c706d73, // 'smpl'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_1                     = 0x31307373, // 'ss01'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_2                     = 0x32307373, // 'ss02'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_3                     = 0x33307373, // 'ss03'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_4                     = 0x34307373, // 'ss04'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_5                     = 0x35307373, // 'ss05'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_6                     = 0x36307373, // 'ss06'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7                     = 0x37307373, // 'ss07'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_8                     = 0x38307373, // 'ss08'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_9                     = 0x39307373, // 'ss09'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_10                    = 0x30317373, // 'ss10'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_11                    = 0x31317373, // 'ss11'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_12                    = 0x32317373, // 'ss12'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_13                    = 0x33317373, // 'ss13'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_14                    = 0x34317373, // 'ss14'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_15                    = 0x35317373, // 'ss15'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_16                    = 0x36317373, // 'ss16'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_17                    = 0x37317373, // 'ss17'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_18                    = 0x38317373, // 'ss18'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_19                    = 0x39317373, // 'ss19'
	DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_20                    = 0x30327373, // 'ss20'
	DWRITE_FONT_FEATURE_TAG_SUBSCRIPT                           = 0x73627573, // 'subs'
	DWRITE_FONT_FEATURE_TAG_SUPERSCRIPT                         = 0x73707573, // 'sups'
	DWRITE_FONT_FEATURE_TAG_SWASH                               = 0x68737773, // 'swsh'
	DWRITE_FONT_FEATURE_TAG_TITLING                             = 0x6c746974, // 'titl'
	DWRITE_FONT_FEATURE_TAG_TRADITIONAL_NAME_FORMS              = 0x6d616e74, // 'tnam'
	DWRITE_FONT_FEATURE_TAG_TABULAR_FIGURES                     = 0x6d756e74, // 'tnum'
	DWRITE_FONT_FEATURE_TAG_TRADITIONAL_FORMS                   = 0x64617274, // 'trad'
	DWRITE_FONT_FEATURE_TAG_THIRD_WIDTHS                        = 0x64697774, // 'twid'
	DWRITE_FONT_FEATURE_TAG_UNICASE                             = 0x63696e75, // 'unic'
	DWRITE_FONT_FEATURE_TAG_SLASHED_ZERO                        = 0x6f72657a, // 'zero'
};

enum DWRITE_FACTORY_TYPE {

    DWRITE_FACTORY_TYPE_SHARED,
    DWRITE_FACTORY_TYPE_ISOLATED
};

typedef enum DWRITE_MEASURING_MODE {

    DWRITE_MEASURING_MODE_NATURAL,
    DWRITE_MEASURING_MODE_GDI_CLASSIC,
    DWRITE_MEASURING_MODE_GDI_NATURAL

} DWRITE_MEASURING_MODE;

enum DWRITE_READING_DIRECTION {
	
	DWRITE_READING_DIRECTION_LEFT_TO_RIGHT,
	DWRITE_READING_DIRECTION_RIGHT_TO_LEFT
};

enum DWRITE_FLOW_DIRECTION {

    DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM
};

enum DWRITE_BREAK_CONDITION {

    DWRITE_BREAK_CONDITION_NEUTRAL,
    DWRITE_BREAK_CONDITION_CAN_BREAK,
    DWRITE_BREAK_CONDITION_MAY_NOT_BREAK,
    DWRITE_BREAK_CONDITION_MUST_BREAK
};

enum DWRITE_PIXEL_GEOMETRY {

    DWRITE_PIXEL_GEOMETRY_FLAT,
    DWRITE_PIXEL_GEOMETRY_RGB,
    DWRITE_PIXEL_GEOMETRY_BGR
};

enum DWRITE_RENDERING_MODE {

    DWRITE_RENDERING_MODE_DEFAULT,
    DWRITE_RENDERING_MODE_ALIASED,
    DWRITE_RENDERING_MODE_CLEARTYPE_GDI_CLASSIC,
    DWRITE_RENDERING_MODE_CLEARTYPE_GDI_NATURAL,
    DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL,
    DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC,
    DWRITE_RENDERING_MODE_OUTLINE
};

enum DWRITE_FONT_WEIGHT {
	
	DWRITE_FONT_WEIGHT_THIN = 100,
	DWRITE_FONT_WEIGHT_EXTRA_LIGHT = 200,
	DWRITE_FONT_WEIGHT_ULTRA_LIGHT = 200,
	DWRITE_FONT_WEIGHT_LIGHT = 300,
	DWRITE_FONT_WEIGHT_NORMAL = 400,
	DWRITE_FONT_WEIGHT_REGULAR = 400,
	DWRITE_FONT_WEIGHT_MEDIUM = 500,
	DWRITE_FONT_WEIGHT_DEMI_BOLD = 600,
	DWRITE_FONT_WEIGHT_SEMI_BOLD = 600,
	DWRITE_FONT_WEIGHT_BOLD = 700,
	DWRITE_FONT_WEIGHT_EXTRA_BOLD = 800,
	DWRITE_FONT_WEIGHT_ULTRA_BOLD = 800,
	DWRITE_FONT_WEIGHT_BLACK = 900,
	DWRITE_FONT_WEIGHT_HEAVY = 900,
	DWRITE_FONT_WEIGHT_EXTRA_BLACK = 95,
	DWRITE_FONT_WEIGHT_ULTRA_BLACK = 950
};

enum DWRITE_FONT_STRETCH {
	
	DWRITE_FONT_STRETCH_UNDEFINED = 0,
	DWRITE_FONT_STRETCH_ULTRA_CONDENSED = 1,
	DWRITE_FONT_STRETCH_EXTRA_CONDENSED = 2,
	DWRITE_FONT_STRETCH_CONDENSED = 3,
	DWRITE_FONT_STRETCH_SEMI_CONDENSED = 4,
	DWRITE_FONT_STRETCH_NORMAL = 5,
	DWRITE_FONT_STRETCH_MEDIUM = 5,
	DWRITE_FONT_STRETCH_SEMI_EXPANDED = 6,
	DWRITE_FONT_STRETCH_EXPANDED = 7,
	DWRITE_FONT_STRETCH_EXTRA_EXPANDED = 8,
	DWRITE_FONT_STRETCH_ULTRA_EXPANDED = 9
};

enum DWRITE_FONT_STYLE {
	
	DWRITE_FONT_STYLE_NORMAL,
	DWRITE_FONT_STYLE_OBLIQUE,
	DWRITE_FONT_STYLE_ITALIC
};

enum DWRITE_FONT_SIMULATIONS {
	
	DWRITE_FONT_SIMULATIONS_NONE    = 0x0000,
	DWRITE_FONT_SIMULATIONS_BOLD    = 0x0001,
	DWRITE_FONT_SIMULATIONS_OBLIQUE = 0x0002
};

enum DWRITE_FONT_FILE_TYPE {
	
	DWRITE_FONT_FILE_TYPE_UNKNOWN,
	DWRITE_FONT_FILE_TYPE_CFF,
	DWRITE_FONT_FILE_TYPE_TRUETYPE,
	DWRITE_FONT_FILE_TYPE_TRUETYPE_COLLECTION,
	DWRITE_FONT_FILE_TYPE_TYPE1_PFM,
	DWRITE_FONT_FILE_TYPE_TYPE1_PFB,
	DWRITE_FONT_FILE_TYPE_VECTOR,
	DWRITE_FONT_FILE_TYPE_BITMAP
};

enum DWRITE_FONT_FACE_TYPE {
	
	DWRITE_FONT_FACE_TYPE_CFF,
	DWRITE_FONT_FACE_TYPE_TRUETYPE,
    DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION,
    DWRITE_FONT_FACE_TYPE_TYPE1,
    DWRITE_FONT_FACE_TYPE_VECTOR,
    DWRITE_FONT_FACE_TYPE_BITMAP,
    DWRITE_FONT_FACE_TYPE_UNKNOWN
};

enum DWRITE_INFORMATIONAL_STRING_ID {
	
	DWRITE_INFORMATIONAL_STRING_NONE,
    DWRITE_INFORMATIONAL_STRING_COPYRIGHT_NOTICE,
    DWRITE_INFORMATIONAL_STRING_VERSION_STRINGS,
    DWRITE_INFORMATIONAL_STRING_TRADEMARK,
    DWRITE_INFORMATIONAL_STRING_MANUFACTURER,
    DWRITE_INFORMATIONAL_STRING_DESIGNER,
    DWRITE_INFORMATIONAL_STRING_DESIGNER_URL,
    DWRITE_INFORMATIONAL_STRING_DESCRIPTION,
    DWRITE_INFORMATIONAL_STRING_FONT_VENDOR_URL,
    DWRITE_INFORMATIONAL_STRING_LICENSE_DESCRIPTION,
    DWRITE_INFORMATIONAL_STRING_LICENSE_INFO_URL,
    DWRITE_INFORMATIONAL_STRING_WIN32_FAMILY_NAMES,
    DWRITE_INFORMATIONAL_STRING_WIN32_SUBFAMILY_NAMES,
    DWRITE_INFORMATIONAL_STRING_PREFERRED_FAMILY_NAMES,
    DWRITE_INFORMATIONAL_STRING_PREFERRED_SUBFAMILY_NAMES,
    DWRITE_INFORMATIONAL_STRING_SAMPLE_TEXT
};

enum DWRITE_TEXT_ALIGNMENT {
	
	DWRITE_TEXT_ALIGNMENT_LEADING,
	DWRITE_TEXT_ALIGNMENT_TRAILING,
	DWRITE_TEXT_ALIGNMENT_CENTER
};

enum DWRITE_PARAGRAPH_ALIGNMENT {
	
	DWRITE_PARAGRAPH_ALIGNMENT_NEAR,
	DWRITE_PARAGRAPH_ALIGNMENT_FAR,
	DWRITE_PARAGRAPH_ALIGNMENT_CENTER
};

enum DWRITE_WORD_WRAPPING {
	
	DWRITE_WORD_WRAPPING_WRAP,
	DWRITE_WORD_WRAPPING_NO_WRAP
};

enum DWRITE_TRIMMING_GRANULARITY {
	
	DWRITE_TRIMMING_GRANULARITY_NONE,
	DWRITE_TRIMMING_GRANULARITY_CHARACTER,
	DWRITE_TRIMMING_GRANULARITY_WORD	
};

enum DWRITE_LINE_SPACING_METHOD {
	
	DWRITE_LINE_SPACING_METHOD_DEFAULT,
	DWRITE_LINE_SPACING_METHOD_UNIFORM
};

enum DWRITE_SCRIPT_SHAPES {
	
	DWRITE_SCRIPT_SHAPES_DEFAULT = 0,
	DWRITE_SCRIPT_SHAPES_NO_VISUAL = 1
};

enum DWRITE_NUMBER_SUBSTITUTION_METHOD {
	
	DWRITE_NUMBER_SUBSTITUTION_METHOD_FROM_CULTURE,
	DWRITE_NUMBER_SUBSTITUTION_METHOD_CONTEXTUAL,
	DWRITE_NUMBER_SUBSTITUTION_METHOD_NONE,
	DWRITE_NUMBER_SUBSTITUTION_METHOD_NATIONAL,
	DWRITE_NUMBER_SUBSTITUTION_METHOD_TRADITIONAL
};

enum DWRITE_TEXTURE_TYPE {
	
	DWRITE_TEXTURE_ALIASED_1x1,
	DWRITE_TEXTURE_CLEARTYPE_3x1
};

struct DWRITE_MATRIX {

    FLOAT m11;
    FLOAT m12;
    FLOAT m21;
    FLOAT m22;
    FLOAT dx;
    FLOAT dy;
};

struct DWRITE_GLYPH_OFFSET {

    FLOAT advanceOffset;
    FLOAT ascenderOffset;
};

struct DWRITE_GLYPH_RUN {

    class IDWriteFontFace * fontFace;
    FLOAT fontEmSize;
    UINT32 glyphCount;
    UINT16 const* glyphIndices;
	FLOAT const* glyphAdvances;
    DWRITE_GLYPH_OFFSET const * glyphOffsets;
    BOOL isSideways;
    UINT32 bidiLevel;
};

struct DWRITE_GLYPH_RUN_DESCRIPTION {
	
	WCHAR const* localeName;
	WCHAR const* string;
	UINT32 stringLength;
	UINT16 const* clusterMap;
	UINT32 textPosition;
};

struct DWRITE_UNDERLINE {
	
	FLOAT width;
	FLOAT thickness;
	FLOAT offset;
	FLOAT runHeight;
	DWRITE_READING_DIRECTION readingDirection;
	DWRITE_FLOW_DIRECTION flowDirection;
	WCHAR const * localeName;
	DWRITE_MEASURING_MODE measuringMode;
};

struct DWRITE_STRIKETHROUGH {

    FLOAT width;
    FLOAT thickness;
    FLOAT offset;
    DWRITE_READING_DIRECTION readingDirection;
    DWRITE_FLOW_DIRECTION flowDirection;
    WCHAR const * localeName;
    DWRITE_MEASURING_MODE measuringMode;
};

struct DWRITE_INLINE_OBJECT_METRICS {

    FLOAT width;
    FLOAT height;
    FLOAT baseline;
    BOOL  supportsSideways;
};

struct DWRITE_OVERHANG_METRICS {

    FLOAT left;
    FLOAT top;
    FLOAT right;
    FLOAT bottom;
};

struct DWRITE_FONT_METRICS {
	
	UINT16 designUnitsPerEm;
	UINT16 ascent;
	UINT16 descent;
	INT16 lineGap;
	UINT16 capHeight;
	UINT16 xHeight;
	INT16 underlinePosition;
	UINT16 underlineThickness;
	INT16 strikethroughPosition;
	UINT16 strikethroughThickness;
};

struct DWRITE_TRIMMING {
	
	DWRITE_TRIMMING_GRANULARITY granularity;
	UINT32 delimiter;
	UINT32 delimiterCount;
};

struct DWRITE_TEXT_METRICS {
	
	FLOAT left;
	FLOAT top;
	FLOAT width;
	FLOAT widthIncludingTrailingWhitespace;
	FLOAT height;
	FLOAT layoutWidth;
	FLOAT layoutHeight;
	UINT32 maxBidiReorderingDepth;
	UINT32 lineCount;
};

struct DWRITE_FONT_FEATURE {
	
	DWRITE_FONT_FEATURE_TAG nameTag;
	UINT32 parameter;
};

struct DWRITE_TEXT_RANGE {
	
	UINT32 startPosition;
	UINT32 length;
};

struct DWRITE_LINE_METRICS {
	
	UINT32 length;	
	UINT32 trailingWhitespaceLength;
	UINT32 newlineLength;
	FLOAT height;
	FLOAT baseline;
	BOOL isTrimmed;
};

struct DWRITE_CLUSTER_METRICS {
	
	FLOAT width;
	UINT16 length;
	UINT16 canWrapLineAfter : 1;
	UINT16 isWhitespace : 1;
	UINT16 isNewline : 1;
	UINT16 isSoftHyphen : 1;
	UINT16 isRightToLeft : 1;
	UINT16 padding : 11;
};

struct DWRITE_HIT_TEST_METRICS {
	
	UINT32 textPosition;
	UINT32 length;
	FLOAT left;
	FLOAT top;
	FLOAT width;
	FLOAT height;
	UINT32 bidiLevel;
	BOOL isText;
	BOOL isTrimmed;
};

struct DWRITE_SCRIPT_ANALYSIS {
	
	UINT16 script;
	DWRITE_SCRIPT_SHAPES shapes;
};

struct DWRITE_LINE_BREAKPOINT {
	
	UINT8 breakConditionBefore  : 2;
	UINT8 breakConditionAfter   : 2;
	UINT8 isWhitespace          : 1;
	UINT8 isSoftHyphen          : 1;
	UINT8 padding               : 2;
};

struct DWRITE_TYPOGRAPHIC_FEATURES {
	
	DWRITE_FONT_FEATURE* features;
	UINT32 featureCount;
};

struct DWRITE_SHAPING_TEXT_PROPERTIES {
	
	UINT16  isShapedAlone       : 1;
	UINT16  reserved            : 15;
};

struct DWRITE_SHAPING_GLYPH_PROPERTIES {
	
	UINT16  justification       : 4;
	UINT16  isClusterStart      : 1;
	UINT16  isDiacritic         : 1;
	UINT16  isZeroWidthSpace    : 1;
	UINT16  reserved            : 9;
};

class IDWritePixelSnapping : public IUnknown {

public:

    virtual HRESULT WINAPI IsPixelSnappingDisabled( void * clientDrawingContext, BOOL * isDisabled ) = 0;
    virtual HRESULT WINAPI GetCurrentTransform( void * clientDrawingContext, DWRITE_MATRIX * transform ) = 0;
    virtual HRESULT WINAPI GetPixelsPerDip( void * clientDrawingContext, FLOAT * pixelsPerDip ) = 0;
};

class IDWriteTextRenderer : public IDWritePixelSnapping {

public:

    virtual HRESULT WINAPI DrawGlyphRun( void * clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_MEASURING_MODE measuringMode,
		DWRITE_GLYPH_RUN const * glyphRun, DWRITE_GLYPH_RUN_DESCRIPTION const * glyphRunDescription, IUnknown* clientDrawingEffect ) = 0;

    virtual HRESULT WINAPI DrawUnderline( void * clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_UNDERLINE const* underline, IUnknown* clientDrawingEffect ) = 0;
    virtual HRESULT WINAPI DrawStrikethrough( void * clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_STRIKETHROUGH const * strikethrough, IUnknown * clientDrawingEffect ) = 0;
    virtual HRESULT WINAPI DrawInlineObject( void * clientDrawingContext, FLOAT originX, FLOAT originY, class IDWriteInlineObject * inlineObject, BOOL isSideways, BOOL isRightToLeft, IUnknown * clientDrawingEffect ) = 0;
};

class IDWriteInlineObject : public IUnknown {

public:

	virtual HRESULT WINAPI Draw( void * clientDrawingContext, IDWriteTextRenderer * renderer, FLOAT originX, FLOAT originY, BOOL isSideways, BOOL isRightToLeft, IUnknown * clientDrawingEffect ) = 0;
    virtual HRESULT WINAPI GetMetrics( DWRITE_INLINE_OBJECT_METRICS * metrics ) = 0;
    virtual HRESULT WINAPI GetOverhangMetrics( DWRITE_OVERHANG_METRICS * overhangs ) = 0;
    virtual HRESULT WINAPI GetBreakConditions( DWRITE_BREAK_CONDITION * breakConditionBefore, DWRITE_BREAK_CONDITION * breakConditionAfter ) = 0;
};

class IDWriteRenderingParams : public IUnknown {

public:

    virtual FLOAT WINAPI GetGamma() = 0;
    virtual FLOAT WINAPI GetEnhancedContrast() = 0;
    virtual FLOAT WINAPI GetClearTypeLevel() = 0;
    virtual DWRITE_PIXEL_GEOMETRY WINAPI GetPixelGeometry() = 0;
    virtual DWRITE_RENDERING_MODE WINAPI GetRenderingMode() = 0;
};

class IDWriteBitmapRenderTarget : public IUnknown {

public:

    virtual HRESULT WINAPI DrawGlyphRun( FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_MEASURING_MODE measuringMode, DWRITE_GLYPH_RUN const * glyphRun,
		IDWriteRenderingParams* renderingParams, COLORREF textColor, RECT * blackBoxRect = NULL ) = 0;

    virtual HDC WINAPI GetMemoryDC() = 0;
    virtual FLOAT WINAPI GetPixelsPerDip() = 0;
    virtual HRESULT WINAPI SetPixelsPerDip( FLOAT pixelsPerDip ) = 0;
    virtual HRESULT WINAPI GetCurrentTransform( DWRITE_MATRIX * transform ) = 0;
    virtual HRESULT WINAPI SetCurrentTransform( DWRITE_MATRIX const* transform ) = 0;
    virtual HRESULT WINAPI GetSize( SIZE * size ) = 0;
    virtual HRESULT WINAPI Resize( UINT32 width, UINT32 height ) = 0;
};

class IDWriteFontFileStream : public IUnknown {

public:
	
	virtual HRESULT WINAPI ReadFileFragment( void const** fragmentStart, UINT64 fileOffset, UINT64 fragmentSize, void** fragmentContext ) = 0;
	virtual void WINAPI ReleaseFileFragment( void* fragmentContext ) = 0;
	virtual HRESULT WINAPI GetFileSize( UINT64* fileSize ) = 0;
	virtual HRESULT WINAPI GetLastWriteTime( UINT64* lastWriteTime ) = 0;
};

class IDWriteFontFileLoader : public IUnknown {

public:
	
	virtual HRESULT WINAPI CreateStreamFromKey( void const* fontFileReferenceKey, UINT32 fontFileReferenceKeySize, IDWriteFontFileStream** fontFileStream ) = 0;
};

class IDWriteFontFile : public IUnknown {
	
	virtual HRESULT WINAPI GetReferenceKey( void const** fontFileReferenceKey, UINT32* fontFileReferenceKeySize ) = 0;
	virtual HRESULT WINAPI GetLoader( IDWriteFontFileLoader** fontFileLoader ) = 0;
	virtual HRESULT WINAPI Analyze( BOOL* isSupportedFontType, DWRITE_FONT_FILE_TYPE* fontFileType, DWRITE_FONT_FACE_TYPE* fontFaceType, UINT32* numberOfFaces ) = 0;
};

class IDWriteFontFileEnumerator : public IUnknown {

public:
	
	virtual HRESULT WINAPI MoveNext( BOOL* hasCurrentFile ) = 0;
	virtual HRESULT WINAPI GetCurrentFontFile( IDWriteFontFile** fontFile ) = 0;
};

class IDWriteFontCollectionLoader : public IUnknown {

public:
	
	virtual HRESULT WINAPI CreateEnumeratorFromKey( class IDWriteFactory* factory, void const* collectionKey, UINT32 collectionKeySize, IDWriteFontFileEnumerator** fontFileEnumerator ) = 0;
};

class IDWriteFontCollection : public IUnknown {

public:
	
	virtual UINT32 WINAPI GetFontFamilyCount() = 0;
	virtual HRESULT WINAPI GetFontFamily( UINT32 index, class IDWriteFontFamily** fontFamily ) = 0;
	virtual HRESULT WINAPI FindFamilyName( WCHAR const* familyName, UINT32* index, BOOL* exists ) = 0;
	virtual HRESULT WINAPI GetFontFromFontFace( IDWriteFontFace* fontFace, class IDWriteFont** font ) = 0;
};

class IDWriteFontList : public IUnknown {

public:
	
	virtual HRESULT WINAPI GetFontCollection( IDWriteFontCollection** fontCollection ) = 0;
	virtual UINT32 WINAPI GetFontCount() = 0;
	virtual HRESULT WINAPI GetFont( UINT32 index, IDWriteFont** font ) = 0;
};

class IDWriteLocalizedStrings : public IUnknown {

public:
	
	virtual UINT32 WINAPI GetCount() = 0;
	virtual HRESULT WINAPI FindLocaleName( WCHAR const* localeName, UINT32* index, BOOL* exists ) = 0;
	virtual HRESULT WINAPI GetLocaleNameLength( UINT32 index, UINT32* length ) = 0;
	virtual HRESULT WINAPI GetLocaleName( UINT32 index, WCHAR* localeName, UINT32 size ) = 0;
	virtual HRESULT WINAPI GetStringLength( UINT32 index, UINT32* length ) = 0;
	virtual HRESULT WINAPI GetString( UINT32 index, WCHAR* stringBuffer, UINT32 size ) = 0;
};

class IDWriteFontFamily : public IDWriteFontList {

public:
	
	virtual HRESULT WINAPI GetFamilyNames( IDWriteLocalizedStrings ** names ) = 0;
	virtual HRESULT WINAPI GetFirstMatchingFont( DWRITE_FONT_WEIGHT  weight, DWRITE_FONT_STRETCH stretch, DWRITE_FONT_STYLE style, IDWriteFont** matchingFont ) = 0;
	virtual HRESULT WINAPI GetMatchingFonts( DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STRETCH stretch, DWRITE_FONT_STYLE style, IDWriteFontList** matchingFonts ) = 0;
};

class IDWriteFont : public IUnknown {

public:
	
	virtual HRESULT WINAPI GetFontFamily( IDWriteFontFamily** fontFamily ) = 0;
	virtual DWRITE_FONT_WEIGHT WINAPI GetWeight() = 0;
	virtual DWRITE_FONT_STRETCH WINAPI GetStretch() = 0;
	virtual DWRITE_FONT_STYLE WINAPI GetStyle() = 0;
	virtual BOOL WINAPI IsSymbolFont() = 0;
	virtual HRESULT WINAPI GetFaceNames( IDWriteLocalizedStrings** names ) = 0;
	virtual HRESULT WINAPI GetInformationalStrings( DWRITE_INFORMATIONAL_STRING_ID informationalStringID, IDWriteLocalizedStrings** informationalStrings, BOOL* exists ) = 0;
	virtual DWRITE_FONT_SIMULATIONS WINAPI GetSimulations() = 0;
	virtual void WINAPI GetMetrics( DWRITE_FONT_METRICS* fontMetrics ) = 0;
	virtual HRESULT WINAPI HasCharacter( UINT32 unicodeValue, BOOL* exists ) = 0;
	virtual HRESULT WINAPI CreateFontFace( IDWriteFontFace** fontFace ) = 0;
};

class IDWriteGdiInterop : public IUnknown {

public:
	
	virtual HRESULT WINAPI CreateFontFromLOGFONT( LOGFONTW const* logFont, IDWriteFont** font ) = 0;
	virtual HRESULT WINAPI ConvertFontToLOGFONT( IDWriteFont* font, LOGFONTW* logFont, BOOL* isSystemFont ) = 0;
	virtual HRESULT WINAPI ConvertFontFaceToLOGFONT( IDWriteFontFace* font, LOGFONTW* logFont ) = 0;
	virtual HRESULT WINAPI CreateFontFaceFromHdc( HDC hdc, IDWriteFontFace** fontFace ) = 0;
	virtual HRESULT WINAPI CreateBitmapRenderTarget( HDC hdc, UINT32 width, UINT32 height, IDWriteBitmapRenderTarget** renderTarget ) = 0;
};

class IDWriteTextFormat : public IUnknown {

public:

	virtual HRESULT WINAPI SetTextAlignment( DWRITE_TEXT_ALIGNMENT textAlignment ) = 0;
	virtual HRESULT WINAPI SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment ) = 0;
	virtual HRESULT WINAPI SetWordWrapping( DWRITE_WORD_WRAPPING wordWrapping ) = 0;
	virtual HRESULT WINAPI SetReadingDirection( DWRITE_READING_DIRECTION readingDirection ) = 0;
	virtual HRESULT WINAPI SetFlowDirection( DWRITE_FLOW_DIRECTION flowDirection ) = 0;
	virtual HRESULT WINAPI SetIncrementalTabStop( FLOAT incrementalTabStop ) = 0;
	virtual HRESULT WINAPI SetTrimming( DWRITE_TRIMMING const* trimmingOptions, IDWriteInlineObject* trimmingSign ) = 0;
	virtual HRESULT WINAPI SetLineSpacing( DWRITE_LINE_SPACING_METHOD lineSpacingMethod, FLOAT lineSpacing, FLOAT baseline ) = 0;
	virtual DWRITE_TEXT_ALIGNMENT WINAPI GetTextAlignment() = 0;
	virtual DWRITE_PARAGRAPH_ALIGNMENT WINAPI GetParagraphAlignment() = 0;
	virtual DWRITE_WORD_WRAPPING WINAPI GetWordWrapping() = 0;
	virtual DWRITE_READING_DIRECTION WINAPI GetReadingDirection() = 0;
	virtual DWRITE_FLOW_DIRECTION WINAPI GetFlowDirection() = 0;
	virtual FLOAT WINAPI GetIncrementalTabStop() PURE;
	virtual HRESULT WINAPI GetTrimming( DWRITE_TRIMMING* trimmingOptions, IDWriteInlineObject** trimmingSign ) = 0;
	virtual HRESULT WINAPI GetLineSpacing( DWRITE_LINE_SPACING_METHOD* lineSpacingMethod, FLOAT* lineSpacing, FLOAT* baseline ) = 0;
	virtual HRESULT WINAPI GetFontCollection( IDWriteFontCollection** fontCollection ) = 0;
	virtual UINT32 WINAPI GetFontFamilyNameLength() = 0;
	virtual HRESULT WINAPI GetFontFamilyName( WCHAR* fontFamilyName, UINT32 nameSize ) = 0;
	virtual DWRITE_FONT_WEIGHT WINAPI GetFontWeight() = 0;
	virtual DWRITE_FONT_STYLE WINAPI GetFontStyle() = 0;
	virtual DWRITE_FONT_STRETCH WINAPI GetFontStretch() = 0;
	virtual FLOAT WINAPI GetFontSize() = 0;
	virtual UINT32 WINAPI GetLocaleNameLength() = 0;
	virtual HRESULT WINAPI GetLocaleName( WCHAR* localeName, UINT32 nameSize ) = 0;
};

class IDWriteTypography : public IUnknown {

public:
	
	virtual HRESULT WINAPI AddFontFeature( DWRITE_FONT_FEATURE fontFeature ) = 0;
	virtual UINT32 WINAPI GetFontFeatureCount() = 0;
	virtual HRESULT WINAPI GetFontFeature( UINT32 fontFeatureIndex, DWRITE_FONT_FEATURE* fontFeature ) = 0;
};

class IDWriteTextLayout : public IDWriteTextFormat {

public:
	
	virtual HRESULT WINAPI SetMaxWidth( FLOAT maxWidth ) = 0;
	virtual HRESULT WINAPI SetMaxHeight( FLOAT maxHeight ) = 0;
	virtual HRESULT WINAPI SetFontCollection( IDWriteFontCollection* fontCollection, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetFontFamilyName( WCHAR const* fontFamilyName, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetFontWeight( DWRITE_FONT_WEIGHT fontWeight, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetFontStyle( DWRITE_FONT_STYLE fontStyle, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetFontStretch( DWRITE_FONT_STRETCH fontStretch, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetFontSize( FLOAT fontSize, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetUnderline( BOOL hasUnderline, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetStrikethrough( BOOL hasStrikethrough, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetDrawingEffect( IUnknown* drawingEffect, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetInlineObject( IDWriteInlineObject* inlineObject, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetTypography( IDWriteTypography* typography, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual HRESULT WINAPI SetLocaleName( WCHAR const* localeName, DWRITE_TEXT_RANGE textRange ) = 0;
	virtual FLOAT WINAPI GetMaxWidth() = 0;
	virtual FLOAT WINAPI GetMaxHeight() = 0;
	virtual HRESULT WINAPI GetFontCollection( UINT32 currentPosition, IDWriteFontCollection** fontCollection, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetFontFamilyNameLength( UINT32 currentPosition, UINT32* nameLength, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetFontFamilyName( UINT32 currentPosition, WCHAR* fontFamilyName, UINT32 nameSize, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetFontWeight( UINT32 currentPosition, DWRITE_FONT_WEIGHT* fontWeight, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetFontStyle( UINT32 currentPosition, DWRITE_FONT_STYLE* fontStyle, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetFontStretch( UINT32 currentPosition, DWRITE_FONT_STRETCH* fontStretch, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetFontSize( UINT32 currentPosition, FLOAT* fontSize, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetUnderline( UINT32 currentPosition, BOOL* hasUnderline, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetStrikethrough( UINT32 currentPosition, BOOL* hasStrikethrough, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetDrawingEffect( UINT32 currentPosition, IUnknown** drawingEffect, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetInlineObject( UINT32 currentPosition, IDWriteInlineObject** inlineObject, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetTypography( UINT32 currentPosition, IDWriteTypography** typography, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetLocaleNameLength( UINT32 currentPosition, UINT32* nameLength, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI GetLocaleName( UINT32 currentPosition, WCHAR* localeName, UINT32 nameSize, DWRITE_TEXT_RANGE* textRange = NULL ) = 0;
	virtual HRESULT WINAPI Draw( void* clientDrawingContext, IDWriteTextRenderer* renderer, FLOAT originX, FLOAT originY ) = 0;
	virtual HRESULT WINAPI GetLineMetrics( DWRITE_LINE_METRICS* lineMetrics, UINT32 maxLineCount, UINT32* actualLineCount ) = 0;
	virtual HRESULT WINAPI GetMetrics( DWRITE_TEXT_METRICS* textMetrics ) = 0;
	virtual HRESULT WINAPI GetOverhangMetrics( DWRITE_OVERHANG_METRICS* overhangs ) = 0;
	virtual HRESULT WINAPI GetClusterMetrics( DWRITE_CLUSTER_METRICS* clusterMetrics, UINT32 maxClusterCount, UINT32* actualClusterCount ) = 0;
	virtual HRESULT WINAPI DetermineMinWidth( FLOAT* minWidth ) = 0;
	virtual HRESULT WINAPI HitTestPoint( FLOAT pointX, FLOAT pointY, BOOL* isTrailingHit, BOOL* isInside, DWRITE_HIT_TEST_METRICS* hitTestMetrics ) = 0;
	virtual HRESULT WINAPI HitTestTextPosition( UINT32 textPosition, BOOL isTrailingHit, FLOAT* pointX, FLOAT* pointY, DWRITE_HIT_TEST_METRICS* hitTestMetrics ) = 0;
	virtual HRESULT WINAPI HitTestTextRange( UINT32 textPosition, UINT32 textLength, FLOAT originX, FLOAT originY, DWRITE_HIT_TEST_METRICS* hitTestMetrics, UINT32 maxHitTestMetricsCount, UINT32* actualHitTestMetricsCount ) = 0;
};

class IDWriteNumberSubstitution : public IUnknown {
};

class IDWriteTextAnalysisSource : public IUnknown {

public:
	
	virtual HRESULT WINAPI GetTextAtPosition( UINT32 textPosition, WCHAR const** textString, UINT32* textLength ) = 0;
	virtual HRESULT WINAPI GetTextBeforePosition( UINT32 textPosition, WCHAR const** textString, UINT32* textLength ) = 0;
	virtual DWRITE_READING_DIRECTION WINAPI GetParagraphReadingDirection() = 0;
	virtual HRESULT WINAPI GetLocaleName( UINT32 textPosition, UINT32* textLength, WCHAR const** localeName ) = 0;
	virtual HRESULT WINAPI GetNumberSubstitution( UINT32 textPosition, UINT32* textLength, IDWriteNumberSubstitution** numberSubstitution ) = 0;
};

class IDWriteTextAnalysisSink : public IUnknown {

public:
	
	virtual HRESULT WINAPI SetScriptAnalysis( UINT32 textPosition, UINT32 textLength, DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis ) = 0;
	virtual HRESULT WINAPI SetLineBreakpoints( UINT32 textPosition, UINT32 textLength, DWRITE_LINE_BREAKPOINT const* lineBreakpoints ) = 0;
	virtual HRESULT WINAPI SetBidiLevel( UINT32 textPosition, UINT32 textLength, UINT8 explicitLevel, UINT8 resolvedLevel ) = 0;
	virtual HRESULT WINAPI SetNumberSubstitution( UINT32 textPosition, UINT32 textLength, IDWriteNumberSubstitution* numberSubstitution ) = 0;
};

class IDWriteTextAnalyzer : public IUnknown {

public:
	
	virtual HRESULT WINAPI AnalyzeScript( IDWriteTextAnalysisSource* analysisSource, UINT32 textPosition, UINT32 textLength, IDWriteTextAnalysisSink* analysisSink ) = 0;
	virtual HRESULT WINAPI AnalyzeBidi( IDWriteTextAnalysisSource* analysisSource, UINT32 textPosition, UINT32 textLength, IDWriteTextAnalysisSink* analysisSink ) = 0;
	virtual HRESULT WINAPI AnalyzeNumberSubstitution( IDWriteTextAnalysisSource* analysisSource, UINT32 textPosition, UINT32 textLength, IDWriteTextAnalysisSink* analysisSink ) = 0;
	virtual HRESULT WINAPI AnalyzeLineBreakpoints( IDWriteTextAnalysisSource* analysisSource, UINT32 textPosition, UINT32 textLength, IDWriteTextAnalysisSink* analysisSink ) = 0;
	virtual HRESULT WINAPI GetGlyphs( WCHAR const* textString, UINT32 textLength, IDWriteFontFace* fontFace, BOOL isSideways, BOOL isRightToLeft,
		DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis, WCHAR const* localeName, IDWriteNumberSubstitution* numberSubstitution,
		DWRITE_TYPOGRAPHIC_FEATURES const** features, UINT32 const* featureRangeLengths, UINT32 featureRanges, UINT32 maxGlyphCount, UINT16* clusterMap,
		DWRITE_SHAPING_TEXT_PROPERTIES* textProps, UINT16* glyphIndices, DWRITE_SHAPING_GLYPH_PROPERTIES* glyphProps, UINT32* actualGlyphCount ) = 0;

	virtual HRESULT WINAPI GetGlyphPlacements( WCHAR const* textString, UINT16 const* clusterMap, DWRITE_SHAPING_TEXT_PROPERTIES* textProps, UINT32 textLength,
		UINT16 const* glyphIndices, DWRITE_SHAPING_GLYPH_PROPERTIES const* glyphProps, UINT32 glyphCount, IDWriteFontFace * fontFace, FLOAT fontEmSize,
		BOOL isSideways, BOOL isRightToLeft, DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis, WCHAR const* localeName, DWRITE_TYPOGRAPHIC_FEATURES const** features,
		UINT32 const* featureRangeLengths, UINT32 featureRanges, FLOAT* glyphAdvances, DWRITE_GLYPH_OFFSET* glyphOffsets ) = 0;

	virtual HRESULT WINAPI GetGdiCompatibleGlyphPlacements( WCHAR const* textString, UINT16 const* clusterMap, DWRITE_SHAPING_TEXT_PROPERTIES* textProps, UINT32 textLength,
		UINT16 const* glyphIndices, DWRITE_SHAPING_GLYPH_PROPERTIES const* glyphProps, UINT32 glyphCount, IDWriteFontFace * fontFace, FLOAT fontEmSize,
		FLOAT pixelsPerDip, DWRITE_MATRIX const* transform, BOOL useGdiNatural, BOOL isSideways, BOOL isRightToLeft, DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis,
		WCHAR const* localeName, DWRITE_TYPOGRAPHIC_FEATURES const** features, UINT32 const* featureRangeLengths, UINT32 featureRanges, FLOAT* glyphAdvances,
		DWRITE_GLYPH_OFFSET* glyphOffsets ) = 0;
};

class IDWriteGlyphRunAnalysis : public IUnknown {

public:
	
	virtual HRESULT WINAPI GetAlphaTextureBounds( DWRITE_TEXTURE_TYPE textureType, RECT* textureBounds ) = 0;
	virtual HRESULT WINAPI CreateAlphaTexture( DWRITE_TEXTURE_TYPE textureType, RECT const* textureBounds, BYTE* alphaValues, UINT32 bufferSize ) = 0;
	virtual HRESULT WINAPI GetAlphaBlendParams( IDWriteRenderingParams* renderingParams, FLOAT* blendGamma, FLOAT* blendEnhancedContrast, FLOAT* blendClearTypeLevel ) = 0;
};

class IDWriteFactory : public IUnknown {

public:
	
	virtual HRESULT WINAPI GetSystemFontCollection( IDWriteFontCollection** fontCollection, BOOL checkForUpdates = FALSE ) = 0;	
	virtual HRESULT WINAPI CreateCustomFontCollection( IDWriteFontCollectionLoader* collectionLoader, void const * collectionKey, UINT32 collectionKeySize, IDWriteFontCollection** fontCollection ) = 0;
	virtual HRESULT WINAPI RegisterFontCollectionLoader( IDWriteFontCollectionLoader* fontCollectionLoader ) = 0;
	virtual HRESULT WINAPI UnregisterFontCollectionLoader( IDWriteFontCollectionLoader* fontCollectionLoader ) = 0;
	virtual HRESULT WINAPI CreateFontFileReference( WCHAR const* filePath, FILETIME const* lastWriteTime, IDWriteFontFile** fontFile ) = 0;
	virtual HRESULT WINAPI CreateCustomFontFileReference( void const* fontFileReferenceKey, UINT32 fontFileReferenceKeySize, IDWriteFontFileLoader* fontFileLoader, IDWriteFontFile** fontFile ) = 0;
	virtual HRESULT WINAPI CreateFontFace( DWRITE_FONT_FACE_TYPE fontFaceType, UINT32 numberOfFiles, IDWriteFontFile* const* fontFiles, UINT32 faceIndex, DWRITE_FONT_SIMULATIONS fontFaceSimulationFlags, IDWriteFontFace** fontFace ) = 0;
	virtual HRESULT WINAPI CreateRenderingParams( IDWriteRenderingParams** renderingParams ) = 0;
	virtual HRESULT WINAPI CreateMonitorRenderingParams( HMONITOR monitor, IDWriteRenderingParams** renderingParams ) = 0;
	virtual HRESULT WINAPI CreateCustomRenderingParams( FLOAT gamma, FLOAT enhancedContrast, FLOAT clearTypeLevel, DWRITE_PIXEL_GEOMETRY pixelGeometry, DWRITE_RENDERING_MODE renderingMode, IDWriteRenderingParams** renderingParams ) = 0;
	virtual HRESULT WINAPI RegisterFontFileLoader( IDWriteFontFileLoader* fontFileLoader ) = 0;
	virtual HRESULT WINAPI UnregisterFontFileLoader( IDWriteFontFileLoader* fontFileLoader ) = 0;
	virtual HRESULT WINAPI CreateTextFormat( WCHAR const* fontFamilyName, IDWriteFontCollection* fontCollection, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch,
        FLOAT fontSize, WCHAR const* localeName, IDWriteTextFormat** textFormat ) = 0;
	virtual HRESULT WINAPI CreateTypography( IDWriteTypography** typography ) = 0;
	virtual HRESULT WINAPI GetGdiInterop( IDWriteGdiInterop** gdiInterop ) = 0;
	virtual HRESULT WINAPI CreateTextLayout( WCHAR const* string, UINT32 stringLength, IDWriteTextFormat* textFormat, FLOAT maxWidth, FLOAT maxHeight, IDWriteTextLayout** textLayout ) = 0;
	virtual HRESULT WINAPI CreateGdiCompatibleTextLayout( WCHAR const* string, UINT32 stringLength, IDWriteTextFormat* textFormat, FLOAT layoutWidth, FLOAT layoutHeight,
        FLOAT pixelsPerDip, DWRITE_MATRIX const* transform, BOOL useGdiNatural, IDWriteTextLayout** textLayout ) = 0;
	virtual HRESULT WINAPI CreateEllipsisTrimmingSign( IDWriteTextFormat* textFormat, IDWriteInlineObject** trimmingSign ) = 0;
	virtual HRESULT WINAPI CreateTextAnalyzer( IDWriteTextAnalyzer** textAnalyzer ) = 0;
	virtual HRESULT WINAPI CreateNumberSubstitution( DWRITE_NUMBER_SUBSTITUTION_METHOD substitutionMethod, WCHAR const* localeName, BOOL ignoreUserOverride, IDWriteNumberSubstitution** numberSubstitution ) = 0;
	virtual HRESULT WINAPI CreateGlyphRunAnalysis( DWRITE_GLYPH_RUN const* glyphRun, FLOAT pixelsPerDip, DWRITE_MATRIX const* transform, DWRITE_RENDERING_MODE renderingMode, DWRITE_MEASURING_MODE measuringMode, FLOAT baselineOriginX, FLOAT baselineOriginY, IDWriteGlyphRunAnalysis** glyphRunAnalysis ) = 0;
};

typedef HRESULT ( WINAPI * DWRITECREATEFACTORY )( DWRITE_FACTORY_TYPE factoryType, const IID & iid, IUnknown ** factory );

}

class DWRenderer : private WindowsNS::IDWriteTextRenderer {

private:

	WindowsNS::IDWriteBitmapRenderTarget *		renderTarget;
	WindowsNS::IDWriteRenderingParams *			renderParams;

	virtual WindowsNS::HRESULT WINAPI QueryInterface( const WindowsNS::IID & riid, void ** ppvObject );
	virtual WindowsNS::ULONG WINAPI AddRef( );
	virtual WindowsNS::ULONG WINAPI Release( );
	virtual WindowsNS::HRESULT WINAPI IsPixelSnappingDisabled( void * clientDrawingContext, WindowsNS::BOOL * isDisabled );
	virtual WindowsNS::HRESULT WINAPI GetCurrentTransform( void * clientDrawingContext, WindowsNS::DWRITE_MATRIX * transform );
	virtual WindowsNS::HRESULT WINAPI GetPixelsPerDip( void * clientDrawingContext, float * pixelsPerDip );
	
    virtual WindowsNS::HRESULT WINAPI DrawGlyphRun( void * clientDrawingContext, float baselineOriginX, float baselineOriginY, WindowsNS::DWRITE_MEASURING_MODE measuringMode,
		WindowsNS::DWRITE_GLYPH_RUN const * glyphRun, WindowsNS::DWRITE_GLYPH_RUN_DESCRIPTION const * glyphRunDescription, WindowsNS::IUnknown * clientDrawingEffect );

    virtual WindowsNS::HRESULT WINAPI DrawUnderline( void * clientDrawingContext, float baselineOriginX, float baselineOriginY,
        WindowsNS::DWRITE_UNDERLINE const * underline, WindowsNS::IUnknown * clientDrawingEffect );

    virtual WindowsNS::HRESULT WINAPI DrawStrikethrough( void * clientDrawingContext, float baselineOriginX, float baselineOriginY,
		WindowsNS::DWRITE_STRIKETHROUGH const * strikethrough, WindowsNS::IUnknown * clientDrawingEffect );

    virtual WindowsNS::HRESULT WINAPI DrawInlineObject( void * clientDrawingContext, float originX, float originY, WindowsNS::IDWriteInlineObject * inlineObject,
		WindowsNS::BOOL isSideways, WindowsNS::BOOL isRightToLeft, WindowsNS::IUnknown * clientDrawingEffect );

public:

	void											SetTarget( WindowsNS::IDWriteBitmapRenderTarget * render_target ) { renderTarget = render_target; }
	void											SetParams( WindowsNS::IDWriteRenderingParams * render_params ) { renderParams = render_params; }
};

class DWImports {

private:

	static DynamicModule *							dwModule;
	static WindowsNS::GUID							GUIDDWrite;
	static DWRenderer								dwRenderer;

public:

	static WindowsNS::DWRITECREATEFACTORY			DWriteCreateFactory;
	static WindowsNS::IDWriteFactory *				DWriteFactory;

	static void										Init( );
	static void										Shutdown( );

	static void										RenderFont( const Str & font_name, float font_pts, Bounds2D & font_bounds, int char_start, int char_range, class Image ** char_set_ptr );
};

class DWFontRenderer {

private:
	
	DWRenderer								d_renderer;
	List< CEGUI::Imageset * >				d_imageSets;

	float									d_fontPts;
	float									d_fontEmDivisor;

	WindowsNS::IDWriteTextFormat *			d_textFormat;
	WindowsNS::IDWriteGdiInterop *			d_gdiInterop;
	WindowsNS::IDWriteBitmapRenderTarget *	d_renderTarget;
	WindowsNS::IDWriteRenderingParams *		d_renderParams;
	WindowsNS::IDWriteFontCollection *		d_fontCollection;
	WindowsNS::IDWriteFontFamily *			d_fontFamily;
	WindowsNS::IDWriteFont *				d_fontInternal;
	WindowsNS::DWRITE_FONT_METRICS			d_fontMetrics;
	
	int										d_bmpWidth;
	int										d_bmpHeight;
	WindowsNS::BITMAP						d_bmpMemComp;
	WindowsNS::HDC							d_hdcMem;
	WindowsNS::HDC							d_hdcMemComp;
	WindowsNS::HBITMAP						d_hbmMemComp;
	WindowsNS::HBRUSH						d_brush;

	void									ResizeDIB( WindowsNS::DWRITE_TEXT_METRICS & glyph_metrics, byte * image_buffer );

public:

											DWFontRenderer( const Str & font_name, float font_pts );
											~DWFontRenderer( );

	CEGUI::utf32							GetCodepointMap( std::map< CEGUI::utf32, CEGUI::FontGlyph > & codepoint_map );
	float									GetAscender( ) { return ( float )( d_fontMetrics.ascent ) / ( float )( d_fontMetrics.designUnitsPerEm ) * d_fontEmDivisor; }
	float									GetDescender( ) { return ( float )( d_fontMetrics.descent ) / ( float )( d_fontMetrics.designUnitsPerEm ) * d_fontEmDivisor; }
	float									GetHeight( ) { return ( float )( ( d_fontMetrics.ascent - d_fontMetrics.descent ) + d_fontMetrics.lineGap ) / ( float )( d_fontMetrics.designUnitsPerEm ) * d_fontEmDivisor; }
	float									GetCapHeight( ) { return ( float )( d_fontMetrics.capHeight ) / ( float )( d_fontMetrics.designUnitsPerEm ) * d_fontEmDivisor; }
	void									Render( const CEGUI::String & font_name, const std::map< CEGUI::utf32, CEGUI::FontGlyph > & codepoint_map, CEGUI::utf32 start_codepoint, CEGUI::utf32 end_codepoint );
};

#endif
#endif
