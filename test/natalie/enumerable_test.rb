require_relative '../spec_helper'

describe 'Enumerable' do
  describe '#map' do
    it 'returns a new array, yielding each item to the block' do
      class Things
        include Enumerable

        def each
          yield 1
          yield 2
          yield 3
        end
      end

      Things.new.map { |i| i * 2 }.should == [2, 4, 6]
    end

    it 'works with a hash' do
      h = { 1 => 2, 3 => 4 }
      h.map { |k, v| [k, v] }.should == [
        [1, 2],
        [3, 4],
      ]
    end
  end

  describe '#detect' do
    i = [1, 2, 3, 4, 5].detect { |i| i > 2 }
    i.should == 3
  end

  describe '#partition' do
    [1, 2, 3, 4].partition(&:odd?).should == [[1, 3], [2, 4]]
    [1, 2, 3, 4].partition { |i| i <= 2 }.should == [[1, 2], [3, 4]]
  end
end
