// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "hfi_property.h"
#include "hfi_buffer_iris2.h"
#include "msm_vidc_buffer_iris2.h"
#include "msm_vidc_buffer.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_debug.h"

static u32 msm_vidc_decoder_bin_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, num_vpp_pipes;
	struct v4l2_format *f;
	bool is_interlaced;
	u32 vpp_delay;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;

	if (!core->capabilities) {
		d_vpr_e("%s: invalid capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	if (inst->decode_vpp_delay.enable)
		vpp_delay = inst->decode_vpp_delay.size;
	else
		vpp_delay = DEFAULT_BSE_VPP_DELAY;
	is_interlaced = false; //TODO: (inst->pic_struct == MSM_VIDC_PIC_STRUCT_MAYBE_INTERLACED);
	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_BIN_H264D(size, width, height,
			is_interlaced, vpp_delay, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_BIN_H265D(size, width, height,
			0, vpp_delay, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_VP9)
		HFI_BUFFER_BIN_VP9D(size, width, height,
			0, num_vpp_pipes);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_comv_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	u32 width, height, out_min_count, vpp_delay;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	if (inst->decode_vpp_delay.enable)
		vpp_delay = inst->decode_vpp_delay.size;
	else
		vpp_delay = DEFAULT_BSE_VPP_DELAY;
	out_min_count = inst->buffers.output.min_count;
	out_min_count = max(vpp_delay + 1, out_min_count);

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_COMV_H264D(size, width, height, out_min_count);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_COMV_H265D(size, width, height, out_min_count);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_non_comv_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	u32 width, height, num_vpp_pipes;
	struct msm_vidc_core* core;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		s_vpr_e(inst->sid, "%s: invalid core capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;

	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_NON_COMV_H264D(size, width, height, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_NON_COMV_H265D(size, width, height, num_vpp_pipes);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_line_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, out_min_count, num_vpp_pipes, vpp_delay;
	struct v4l2_format *f;
	bool is_opb;
	u32 pixelformat;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		d_vpr_e("%s: invalid capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;

	pixelformat = inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat;
	if (pixelformat == MSM_VIDC_FMT_NV12 ||
		pixelformat == MSM_VIDC_FMT_P010)
		is_opb = true;
	else
		is_opb = false;

	if (inst->decode_vpp_delay.enable)
		vpp_delay = inst->decode_vpp_delay.size;
	else
		vpp_delay = DEFAULT_BSE_VPP_DELAY;

	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	out_min_count = inst->buffers.output.min_count;
	out_min_count = max(vpp_delay + 1, out_min_count);

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_LINE_H264D(size, width, height, out_min_count,
			is_opb, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_LINE_H265D(size, width, height, is_opb,
			num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_VP9)
		HFI_BUFFER_LINE_VP9D(size, width, height, out_min_count,
			is_opb, num_vpp_pipes);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_persist_size_iris2(struct msm_vidc_inst *inst)
{
	u32 size = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_PERSIST_H264D(size);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_PERSIST_H265D(size);
	else if (inst->codec == MSM_VIDC_VP9)
		HFI_BUFFER_PERSIST_VP9D(size);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

/* encoder internal buffers */
static u32 msm_vidc_encoder_bin_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, num_vpp_pipes, stage;
	struct v4l2_format *f;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		s_vpr_e(inst->sid, "%s: invalid core capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	stage = inst->capabilities->cap[STAGE].value;
	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_BIN_H264E(size, width, height, stage, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_BIN_H265E(size, width, height, stage, num_vpp_pipes);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_comv_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	u32 width, height, num_ref;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264) {
		// TODO: replace zeros with appropriate variables
		HFI_IRIS2_ENC_RECON_BUF_COUNT(num_ref, 0, 0, 0, 0, 0,
			HFI_CODEC_ENCODE_AVC);
		HFI_BUFFER_COMV_H264E(size, width, height, num_ref);
	} else if (inst->codec == MSM_VIDC_HEVC) {
		// TODO: replace zeros with appropriate variables
		HFI_IRIS2_ENC_RECON_BUF_COUNT(num_ref, 0, 0, 0, 0, 0,
			HFI_CODEC_ENCODE_HEVC);
		HFI_BUFFER_COMV_H265E(size, width, height, num_ref);
	}

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_non_comv_size_iris2(struct msm_vidc_inst* inst)
{
	struct msm_vidc_core* core;
	u32 size = 0;
	u32 width, height, num_vpp_pipes;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		s_vpr_e(inst->sid, "%s: invalid core capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_NON_COMV_H264E(size, width, height, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_NON_COMV_H265E(size, width, height, num_vpp_pipes);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_line_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, pixelformat, num_vpp_pipes;
	bool is_tenbit = false;
	struct v4l2_format *f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		s_vpr_e(inst->sid, "%s: invalid core capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	pixelformat = f->fmt.pix_mp.pixelformat;
	if (pixelformat == MSM_VIDC_FMT_P010 ||
		pixelformat == MSM_VIDC_FMT_TP10C)
		is_tenbit = true;
	else
		is_tenbit = false;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_LINE_H264E(size, width, height, is_tenbit, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_LINE_H265E(size, width, height, is_tenbit, num_vpp_pipes);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_dpb_size_iris2(struct msm_vidc_inst *inst)
{
	u32 size = 0;
	u32 width, height, pixelformat;
	struct v4l2_format *f;
	bool is_tenbit;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	pixelformat = f->fmt.pix_mp.pixelformat;
	if (pixelformat == MSM_VIDC_FMT_P010 ||
		pixelformat == MSM_VIDC_FMT_TP10C)
		is_tenbit = true;
	else
		is_tenbit = false;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_DPB_H264E(size, width, height);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_DPB_H265E(size, width, height, is_tenbit);

	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_arp_size_iris2(struct msm_vidc_inst *inst)
{
	u32 size = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	HFI_BUFFER_ARP_ENC(size);
	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_vpss_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	bool ds_enable, rot_enable, flip_enable, is_tenbit;
	u32 width, height, pixelformat;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}
	ds_enable = false; // TODO: fixme
	rot_enable = false; // TODO: fixme
	flip_enable = false; // TODO: fixme
	is_tenbit = false;

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	pixelformat = f->fmt.pix_mp.pixelformat;
	if (pixelformat == MSM_VIDC_FMT_P010 ||
		pixelformat == MSM_VIDC_FMT_TP10C)
		is_tenbit = true;
	else
		is_tenbit = false;

	HFI_BUFFER_VPSS_ENC(size, width, height, ds_enable,
		rot_enable, flip_enable, is_tenbit);
	s_vpr_l(inst->sid, "%s: size %d\n", __func__, size);
	return size;
}

int msm_buffer_size_iris2(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int size = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	if (is_decode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
			size = msm_vidc_decoder_input_size(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT:
			size = msm_vidc_decoder_output_size(inst);
			break;
		case MSM_VIDC_BUF_INPUT_META:
			size = msm_vidc_decoder_input_meta_size(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT_META:
			size = msm_vidc_decoder_output_meta_size(inst);
			break;
		case MSM_VIDC_BUF_BIN:
			size = msm_vidc_decoder_bin_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_COMV:
			size = msm_vidc_decoder_comv_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_NON_COMV:
			size = msm_vidc_decoder_non_comv_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_LINE:
			size = msm_vidc_decoder_line_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_PERSIST:
			size = msm_vidc_decoder_persist_size_iris2(inst);
			break;
		default:
			break;
		}
	} else if (is_encode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
			size = msm_vidc_encoder_input_size(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT:
			size = msm_vidc_encoder_output_size(inst);
			break;
		case MSM_VIDC_BUF_INPUT_META:
			size = msm_vidc_encoder_input_meta_size(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT_META:
			size = msm_vidc_encoder_output_meta_size(inst);
			break;
		case MSM_VIDC_BUF_BIN:
			size = msm_vidc_encoder_bin_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_COMV:
			size = msm_vidc_encoder_comv_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_NON_COMV:
			size = msm_vidc_encoder_non_comv_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_LINE:
			size = msm_vidc_encoder_line_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_DPB:
			size = msm_vidc_encoder_dpb_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_ARP:
			size = msm_vidc_encoder_arp_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_VPSS:
			size = msm_vidc_encoder_vpss_size_iris2(inst);
			break;
		default:
			break;
		}
	}

	return size;
}

int msm_buffer_min_count_iris2(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int count = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (is_decode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
		case MSM_VIDC_BUF_INPUT_META:
			count = msm_vidc_input_min_count(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT:
		case MSM_VIDC_BUF_OUTPUT_META:
			count = msm_vidc_output_min_count(inst);
			break;
		case MSM_VIDC_BUF_BIN:
		case MSM_VIDC_BUF_COMV:
		case MSM_VIDC_BUF_NON_COMV:
		case MSM_VIDC_BUF_LINE:
		case MSM_VIDC_BUF_PERSIST:
			count = 1;
			break;
		//todo: add DPB
		default:
			break;
		}
	} else if (is_encode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
		case MSM_VIDC_BUF_INPUT_META:
			count = msm_vidc_input_min_count(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT:
		case MSM_VIDC_BUF_OUTPUT_META:
			count = msm_vidc_output_min_count(inst);
			break;
		case MSM_VIDC_BUF_BIN:
		case MSM_VIDC_BUF_COMV:
		case MSM_VIDC_BUF_NON_COMV:
		case MSM_VIDC_BUF_LINE:
		case MSM_VIDC_BUF_DPB:
		//todo: add DPB count
		case MSM_VIDC_BUF_ARP:
		case MSM_VIDC_BUF_VPSS:
			count = 1;
			break;
		default:
			break;
		}
	}

	return count;
}

int msm_buffer_extra_count_iris2(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int count = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
	case MSM_VIDC_BUF_INPUT_META:
		count = msm_vidc_input_extra_count(inst);
		break;
	case MSM_VIDC_BUF_OUTPUT:
	case MSM_VIDC_BUF_OUTPUT_META:
		count = msm_vidc_output_extra_count(inst);
		break;
	default:
		break;
	}

	return count;
}