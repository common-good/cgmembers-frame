<?php
/**
 * Phinx
 *
 * (The MIT license)
 * Copyright (c) 2019 John Ridgway
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated * documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * @package    Phinx
 * @subpackage Phinx\Db\Adapter
 */
namespace Phinx\Db\Adapter;

use Phinx\Db\Table;
use Phinx\Db\Table\Column;
use Phinx\Db\Table\ForeignKey;
use Phinx\Db\Table\Index;

/**
 * Phinx CGMySQL Adapter.
 *
 * @author Rob Morgan <robbym@gmail.com>
 */
class CGMysqlAdapter extends MysqlAdapter
{
  /**
   * {@inheritdoc}
   */
  public function getSqlType($type, $limit = null)
  {
    switch ($type) {
    case 'tinyint':
      return ['name' => 'tinyint'];
    case 'bigint':
      return ['name' => 'bigint'];
    default:
      return parent::getSqlType($type, $limit);
    }
  }

  /**
   * Returns MySQL column types (inherited and MySQL specified).
   * @return array
   */
  public function getColumnTypes()
  {
    return array_merge(parent::getColumnTypes(), ['enum', 'set', 'year', 'json']);
  }
}
