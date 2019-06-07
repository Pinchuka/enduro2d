/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"

namespace e2d::path
{
    [[nodiscard]] str combine(str_view lhs, str_view rhs);

    [[nodiscard]] str remove_filename(str_view path);
    [[nodiscard]] str remove_extension(str_view path);

    [[nodiscard]] str replace_filename(str_view path, str_view filename);
    [[nodiscard]] str replace_extension(str_view path, str_view extension);

    [[nodiscard]] str stem(str_view path);
    [[nodiscard]] str filename(str_view path);
    [[nodiscard]] str extension(str_view path);
    [[nodiscard]] str parent_path(str_view path);

    [[nodiscard]] bool is_absolute(str_view path) noexcept;
    [[nodiscard]] bool is_relative(str_view path) noexcept;
}
