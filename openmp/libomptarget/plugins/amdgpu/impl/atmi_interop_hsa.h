//===--- amdgpu/impl/atmi_interop_hsa.h --------------------------- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef INCLUDE_ATMI_INTEROP_HSA_H_
#define INCLUDE_ATMI_INTEROP_HSA_H_

#include "atmi_runtime.h"
#include "hsa_api.h"
#include "internal.h"

#include <map>
#include <string>

extern "C" {

/** \defgroup interop_hsa_functions ATMI-HSA Interop
 *  @{
 */

/**
 * @brief Get the device address and size of an HSA global symbol
 *
 * @detail Use this function to query the device address and size of an HSA
 * global symbol.
 * The symbol can be set at by the compiler or by the application writer in a
 * language-specific manner. This function is meaningful only after calling one
 * of the @p atmi_module_register functions.
 *
 * @param[in] place The ATMI memory place
 *
 * @param[in] symbol Pointer to a non-NULL global symbol name
 *
 * @param[in] var_addr Pointer to a non-NULL @p void* variable that will
 * hold the device address of the global symbol object.
 *
 * @param[in] var_size Pointer to a non-NULL @p uint variable that will
 * hold the size of the global symbol object.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR If @p symbol, @p var_addr or @p var_size are
 * invalid
 * location in the current node, or if ATMI is not initialized.
 */
hsa_status_t interop_hsa_get_symbol_info(
    const std::map<std::string, atl_symbol_info_t> &SymbolInfoTable,
    int DeviceId, const char *symbol, void **var_addr, unsigned int *var_size);

hsa_status_t interop_hsa_get_kernel_info(
    const std::map<std::string, atl_kernel_info_t> &KernelInfoTable,
    int DeviceId, const char *kernel_name, hsa_executable_symbol_info_t info,
    uint32_t *value);

}

#endif // INCLUDE_ATMI_INTEROP_HSA_H_
