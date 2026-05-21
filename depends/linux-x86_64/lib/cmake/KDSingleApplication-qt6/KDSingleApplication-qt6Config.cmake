# This file is part of KDSingleApplication.
#
# SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
include(CMakeFindDependencyMacro)

find_dependency(Qt6Widgets REQUIRED)
find_dependency(Qt6Network REQUIRED)

# Add the targets file
include("${CMAKE_CURRENT_LIST_DIR}/KDSingleApplication-qt6Targets.cmake")
