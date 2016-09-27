#ifndef ZIMG_PREVIEW_CONVERTER_H_INCLUDED
#define ZIMG_PREVIEW_CONVERTER_H_INCLUDED

#include <QObject>
#include <QPixmap>

#include "../common/aligned_vector.h"
#include "zimg_helpers.h"

class SettingsManager;

class ZimgPreviewConverter : public QObject
{
	Q_OBJECT

public:

	ZimgPreviewConverter(SettingsManager * a_pSettingsManager,
		QObject * a_pParent = nullptr, const VSAPI * a_cpVSAPI = nullptr);
	virtual ~ZimgPreviewConverter();

	void setVSAPI(const VSAPI * a_cpVSAPI);

	QPixmap pixmap(const VSFrameRef * a_cpFrameRef);

public slots:

	void slotResetSettings();

signals:

	void signalWriteLogMessage(int a_messageType, const QString & a_message);

private:

	bool translateColorFamily(VSColorFamily a_vsColorFamily,
		zimg_color_family_e * a_pZimgColorFamily,
		zimg_matrix_coefficients_e * a_pZimgMatrixCoefficients);

	bool translatePixelType(const VSFormat * a_cpVSFormat,
		zimg_pixel_type_e * a_pZimgPixelType);

	bool translateFormat(const VSFormat * a_cpVSFormat,
		zimg_image_format * a_pZimgFormat);

	bool importFrameProps(const VSMap * a_cpProps,
		zimg_image_format * a_pZimgFormat, bool * a_pIsInterlaced);

	void checkZimgError();

	void inpaintError(QPixmap & a_pixmap);

	QString m_error;

	SettingsManager * m_pSettingsManager;

	const VSAPI * m_cpVSAPI;

	zimg_image_format m_inFormat;
	zimg_image_buffer m_inZimgBuffer;
	zimg_filter_graph_callback m_fpUnpackCallback;
	YUY2UnpackData m_yuy2UnpackData;
	void * m_pUnpackData;
	vsedit::aligned_vector<uint8_t> m_inBuffer;
	zimg_image_format m_outFormat;
	zimg_image_buffer m_outZimgBuffer;
	BGR32PackData m_bgr32PackData;
	vsedit::aligned_vector<uint8_t> m_tempBuffer;
	vsedit::aligned_vector<uint8_t> m_outBuffer;
	vsedit::aligned_vector<uint8_t> m_bgr32Buffer;
	zimg_filter_graph * m_pFilterGraph;
	zimg_graph_builder_params m_graphBuilderParams;
};

#endif // ZIMG_PREVIEW_CONVERTER_H_INCLUDED
